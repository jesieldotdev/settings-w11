/*
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "appsmodel.h"

#include <KApplicationTrader>
#include <KService>

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QProcess>
#include <QRegularExpression>
#include <QSet>
#include <QStandardPaths>

#include <algorithm>

namespace
{
// processo com saída em inglês (para ler) que se apaga sozinho ao terminar
QProcess *process(QObject *parent)
{
    auto *p = new QProcess(parent);
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    env.insert(QStringLiteral("LC_ALL"), QStringLiteral("C"));
    p->setProcessEnvironment(env);
    QObject::connect(p, &QProcess::finished, p, &QObject::deleteLater);
    QObject::connect(p, &QProcess::errorOccurred, p, [p](QProcess::ProcessError error) {
        if (error == QProcess::FailedToStart) {
            p->deleteLater();
        }
    });
    return p;
}

// sem acentos e minúsculo, para a pesquisa
QString fold(const QString &text)
{
    QString out;
    for (const QChar c : text.normalized(QString::NormalizationForm_D)) {
        if (c.category() != QChar::Mark_NonSpacing) {
            out += c.toLower();
        }
    }
    return out;
}

// "486.7 MB" (flatpak) -> bytes
qint64 parseSize(QString text)
{
    text.replace(QChar(0xa0), QLatin1Char(' '));
    static const QRegularExpression re(QStringLiteral("([0-9.]+)\\s*(bytes|kB|MB|GB|TB)"));
    const auto m = re.match(text);
    if (!m.hasMatch()) {
        return -1;
    }
    const QString unit = m.captured(2);
    const double mult = unit == QLatin1String("kB") ? 1e3 : unit == QLatin1String("MB") ? 1e6 : unit == QLatin1String("GB") ? 1e9 : unit == QLatin1String("TB") ? 1e12 : 1;
    return qint64(m.captured(1).toDouble() * mult);
}

// pacotes que, se saírem junto, quebram a sessão
bool isCore(const QString &pkg)
{
    static const QRegularExpression re(QStringLiteral("^(plasma-(workspace|desktop|systemsettings)|kwin.*|sddm|systemd.*|kernel.*|dnf.*|rpm|NetworkManager|pipewire|wireplumber|glibc|bash|sudo|polkit)$"));
    return re.match(pkg).hasMatch();
}
}

AppsModel::AppsModel(QObject *parent)
    : QAbstractListModel(parent)
{
    reload();
}

QString AppsModel::kindName(Kind kind)
{
    switch (kind) {
    case Rpm: return QStringLiteral("Sistema (RPM)");
    case Flatpak: return QStringLiteral("Flatpak");
    case Snap: return QStringLiteral("Snap");
    case AppImage: return QStringLiteral("AppImage");
    case Shortcut: return QStringLiteral("Atalho");
    }
    return {};
}

int AppsModel::rowCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : m_visible.size();
}

QVariant AppsModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() >= m_visible.size()) {
        return {};
    }
    const App &a = m_apps.at(m_visible.at(index.row()));
    switch (role) {
    case Qt::DisplayRole:
    case NameRole: return a.name;
    case IconRole: return a.icon;
    case KindRole: return a.kind;
    case KindNameRole: return kindName(a.kind);
    case PackageRole: return a.package;
    case VersionRole: return a.version;
    case PublisherRole: return a.publisher;
    case SizeRole: return a.size;
    case InstalledRole: return a.installed;
    case DesktopRole: return a.desktop;
    case BusyRole: return a.busy;
    }
    return {};
}

QHash<int, QByteArray> AppsModel::roleNames() const
{
    return {
        {NameRole, "name"},
        {IconRole, "iconName"},
        {KindRole, "kind"},
        {KindNameRole, "kindName"},
        {PackageRole, "package"},
        {VersionRole, "version"},
        {PublisherRole, "publisher"},
        {SizeRole, "size"},
        {InstalledRole, "installed"},
        {DesktopRole, "desktop"},
        {BusyRole, "busy"},
    };
}

void AppsModel::setFilterText(const QString &text)
{
    if (text != m_filterText) {
        m_filterText = text;
        rebuild();
        Q_EMIT filterChanged();
    }
}

void AppsModel::setSource(const QString &source)
{
    if (source != m_source) {
        m_source = source;
        rebuild();
        Q_EMIT filterChanged();
    }
}

void AppsModel::setSortBy(const QString &sortBy)
{
    if (sortBy != m_sortBy) {
        m_sortBy = sortBy;
        rebuild();
        Q_EMIT filterChanged();
    }
}

void AppsModel::reload()
{
    if (m_pending > 0) {
        return;
    }
    loadServices();
    m_pending = 3;
    Q_EMIT loadingChanged();
    queryRpm();
    queryFlatpak();
    querySnap();
}

void AppsModel::loadServices()
{
    m_apps.clear();
    const QString home = QDir::homePath();
    const QString localApps = QStandardPaths::writableLocation(QStandardPaths::ApplicationsLocation);
    const auto services = KApplicationTrader::query([](const KService::Ptr &s) {
        return !s->noDisplay() && !s->exec().isEmpty();
    });
    for (const KService::Ptr &s : services) {
        App a;
        a.name = s->name();
        a.icon = s->icon();
        a.desktop = s->entryPath();
        const QString flatpak = s->property<QString>(QStringLiteral("X-Flatpak"));
        const QString snap = s->property<QString>(QStringLiteral("X-SnapInstanceName"));
        if (!flatpak.isEmpty() || a.desktop.contains(QLatin1String("/flatpak/exports/"))) {
            a.kind = Flatpak;
            a.package = !flatpak.isEmpty() ? flatpak : QFileInfo(a.desktop).completeBaseName();
            a.flatpakInstallation = a.desktop.startsWith(home) ? QStringLiteral("user") : QStringLiteral("system");
        } else if (!snap.isEmpty() || a.desktop.contains(QLatin1String("/snapd/desktop/"))) {
            a.kind = Snap;
            a.package = !snap.isEmpty() ? snap : QFileInfo(a.desktop).completeBaseName().section(QLatin1Char('_'), 0, 0);
        } else {
            a.kind = Rpm; // até o rpm dizer que não é dele
            a.lookup = QFileInfo(a.desktop).canonicalFilePath();
            if (a.desktop.startsWith(localApps)) {
                // cópia local de um atalho do sistema (ex.: o do Dolphin do dolphin-w11)
                const QString rel = QStringLiteral("applications/") + QDir(localApps).relativeFilePath(a.desktop);
                const QStringList all = QStandardPaths::locateAll(QStandardPaths::GenericDataLocation, rel);
                a.lookup = all.size() > 1 ? all.at(1) : QString();
            }
            if (a.lookup.isEmpty()) {
                const QString program = s->exec().section(QLatin1Char(' '), 0, 0).remove(QLatin1Char('"'));
                if (program.endsWith(QLatin1String(".AppImage"), Qt::CaseInsensitive) || program.endsWith(QLatin1String(".appimage"))) {
                    a.kind = AppImage;
                    a.package = program;
                    const QFileInfo fi(program);
                    a.size = fi.exists() ? fi.size() : -1;
                    a.installed = fi.lastModified();
                } else {
                    a.kind = Shortcut;
                    a.installed = QFileInfo(a.desktop).lastModified();
                }
            }
        }
        m_apps.append(a);
    }
}

void AppsModel::queryRpm()
{
    QStringList files;
    for (const App &a : std::as_const(m_apps)) {
        if (a.kind == Rpm) {
            files << a.lookup;
        }
    }
    files.removeDuplicates();
    if (files.isEmpty()) {
        finishOne();
        return;
    }
    auto *p = process(this);
    connect(p, &QProcess::finished, this, [this, p, files]() {
        // uma linha por arquivo do pacote dono; só interessam os .desktop pedidos
        const QSet<QString> wanted(files.cbegin(), files.cend());
        QHash<QString, QStringList> owner;
        const auto lines = QString::fromUtf8(p->readAllStandardOutput()).split(QLatin1Char('\n'));
        for (const QString &line : lines) {
            const QStringList f = line.split(QLatin1Char('\t'));
            if (f.size() == 6 && wanted.contains(f.at(0)) && !owner.contains(f.at(0))) {
                owner.insert(f.at(0), f);
            }
        }
        QSet<QString> seen;
        for (int i = 0; i < m_apps.size(); ++i) {
            App &a = m_apps[i];
            if (a.kind != Rpm) {
                continue;
            }
            const QStringList f = owner.value(a.lookup);
            if (f.isEmpty()) {
                a.kind = Shortcut; // ninguém instalou: atalho solto
                a.installed = QFileInfo(a.desktop).lastModified();
                continue;
            }
            a.package = f.at(1);
            a.version = f.at(2);
            a.size = f.at(3).toLongLong();
            a.publisher = f.at(4) == QLatin1String("(none)") ? QString() : f.at(4);
            a.installed = QDateTime::fromSecsSinceEpoch(f.at(5).toLongLong());
        }
        // um pacote com vários atalhos aparece uma vez só, com o nome do app "principal"
        QHash<QString, int> best;
        for (int i = 0; i < m_apps.size(); ++i) {
            const App &a = m_apps.at(i);
            if (a.kind != Rpm) {
                continue;
            }
            const auto it = best.constFind(a.package);
            const bool matches = QFileInfo(a.desktop).fileName().contains(a.package, Qt::CaseInsensitive);
            if (it == best.constEnd()) {
                best.insert(a.package, i);
            } else if (matches && !QFileInfo(m_apps.at(*it).desktop).fileName().contains(a.package, Qt::CaseInsensitive)) {
                best.insert(a.package, i);
            }
        }
        QList<App> kept;
        for (int i = 0; i < m_apps.size(); ++i) {
            if (m_apps.at(i).kind != Rpm || best.value(m_apps.at(i).package) == i) {
                kept << m_apps.at(i);
            }
        }
        m_apps = kept;
        finishOne();
    });
    p->start(QStringLiteral("rpm"),
             QStringList{QStringLiteral("-qf"), QStringLiteral("--qf"),
                         QStringLiteral("[%{FILENAMES}\\t%{=NAME}\\t%{=VERSION}\\t%{=SIZE}\\t%{=VENDOR}\\t%{=INSTALLTIME}\\n]")}
                 + files);
    if (!p->waitForStarted(2000)) { // sem rpm (outra distribuição): ficam como atalhos
        for (App &a : m_apps) {
            if (a.kind == Rpm) {
                a.kind = Shortcut;
            }
        }
        finishOne();
    }
}

void AppsModel::queryFlatpak()
{
    auto *p = process(this);
    connect(p, &QProcess::finished, this, [this, p]() {
        const auto lines = QString::fromUtf8(p->readAllStandardOutput()).split(QLatin1Char('\n'));
        for (const QString &line : lines) {
            const QStringList f = line.split(QLatin1Char('\t'));
            if (f.size() < 5) {
                continue;
            }
            for (App &a : m_apps) {
                if (a.kind == Flatpak && a.package == f.at(0)) {
                    a.version = f.at(1);
                    a.size = parseSize(f.at(2));
                    a.publisher = f.at(3);
                    a.flatpakInstallation = f.at(4);
                    const QString base = f.at(4) == QLatin1String("user") ? QDir::homePath() + QStringLiteral("/.local/share/flatpak") : QStringLiteral("/var/lib/flatpak");
                    a.installed = QFileInfo(base + QStringLiteral("/app/") + f.at(0)).lastModified();
                }
            }
        }
        finishOne();
    });
    p->start(QStringLiteral("flatpak"), {QStringLiteral("list"), QStringLiteral("--app"), QStringLiteral("--columns=application,version,size,origin,installation")});
    if (!p->waitForStarted(2000)) {
        finishOne();
    }
}

void AppsModel::querySnap()
{
    auto *p = process(this);
    connect(p, &QProcess::finished, this, [this, p]() {
        const auto lines = QString::fromUtf8(p->readAllStandardOutput()).split(QLatin1Char('\n')).mid(1);
        for (const QString &line : lines) {
            const QStringList f = line.simplified().split(QLatin1Char(' '));
            if (f.size() < 5) {
                continue;
            }
            for (App &a : m_apps) {
                if (a.kind == Snap && a.package == f.at(0)) {
                    a.version = f.at(1);
                    a.publisher = QString(f.at(4)).remove(QLatin1Char('*')).remove(QChar(0x2713));
                    a.installed = QFileInfo(QStringLiteral("/var/lib/snapd/snaps")).lastModified();
                }
            }
        }
        finishOne();
    });
    p->start(QStringLiteral("snap"), {QStringLiteral("list")});
    if (!p->waitForStarted(2000)) {
        finishOne();
    }
}

void AppsModel::finishOne()
{
    if (--m_pending > 0) {
        return;
    }
    rebuild();
    Q_EMIT loadingChanged();
}

void AppsModel::rebuild()
{
    if (m_pending > 0) {
        return;
    }
    beginResetModel();
    m_visible.clear();
    const QString needle = fold(m_filterText.trimmed());
    for (int i = 0; i < m_apps.size(); ++i) {
        const App &a = m_apps.at(i);
        if (!needle.isEmpty() && !fold(a.name).contains(needle) && !fold(a.package).contains(needle)) {
            continue;
        }
        if (m_source == QLatin1String("rpm") && a.kind != Rpm) continue;
        if (m_source == QLatin1String("flatpak") && a.kind != Flatpak) continue;
        if (m_source == QLatin1String("snap") && a.kind != Snap) continue;
        if (m_source == QLatin1String("outros") && a.kind != AppImage && a.kind != Shortcut) continue;
        m_visible << i;
    }
    const auto byName = [this](int l, int r) {
        return QString::localeAwareCompare(m_apps.at(l).name, m_apps.at(r).name) < 0;
    };
    if (m_sortBy == QLatin1String("name-desc")) {
        std::sort(m_visible.begin(), m_visible.end(), [&](int l, int r) { return byName(r, l); });
    } else if (m_sortBy == QLatin1String("size")) {
        std::sort(m_visible.begin(), m_visible.end(), [&](int l, int r) { return m_apps.at(l).size > m_apps.at(r).size; });
    } else if (m_sortBy == QLatin1String("date")) {
        std::sort(m_visible.begin(), m_visible.end(), [&](int l, int r) { return m_apps.at(l).installed > m_apps.at(r).installed; });
    } else {
        std::sort(m_visible.begin(), m_visible.end(), byName);
    }
    endResetModel();
    Q_EMIT countsChanged();
}

AppsModel::App *AppsModel::findByDesktop(const QString &desktop)
{
    for (App &a : m_apps) {
        if (a.desktop == desktop) {
            return &a;
        }
    }
    return nullptr;
}

void AppsModel::setBusy(const QString &desktop, bool busy)
{
    if (App *a = findByDesktop(desktop)) {
        a->busy = busy;
    }
    for (int row = 0; row < m_visible.size(); ++row) {
        if (m_apps.at(m_visible.at(row)).desktop == desktop) {
            Q_EMIT dataChanged(index(row), index(row), {BusyRole});
        }
    }
}

void AppsModel::launch(int row)
{
    if (row >= 0 && row < m_visible.size()) {
        QProcess::startDetached(QStringLiteral("kioclient"), {QStringLiteral("exec"), m_apps.at(m_visible.at(row)).desktop});
    }
}

void AppsModel::planUninstall(int row)
{
    if (row < 0 || row >= m_visible.size()) {
        return;
    }
    const App a = m_apps.at(m_visible.at(row));
    if (a.kind != Rpm) {
        Q_EMIT planReady(a.desktop, true, {}, {});
        return;
    }
    // simula no dnf (não precisa de senha) para mostrar o que sai junto
    auto *p = process(this);
    connect(p, &QProcess::finished, this, [this, p, a]() {
        const QString out = QString::fromUtf8(p->readAllStandardOutput() + p->readAllStandardError());
        if (out.contains(QLatin1String("Failed to resolve")) || !out.contains(QLatin1String("Removing"))) {
            Q_EMIT planReady(a.desktop, false, {},
                             QStringLiteral("Este aplicativo faz parte do sistema e não pode ser desinstalado."));
            return;
        }
        QStringList also;
        bool inRemoving = false;
        for (const QString &line : out.split(QLatin1Char('\n'))) {
            if (line.startsWith(QLatin1String("Removing"))) {
                inRemoving = true;
                continue;
            }
            if (!line.startsWith(QLatin1Char(' '))) {
                inRemoving = false;
                continue;
            }
            const QString name = line.simplified().section(QLatin1Char(' '), 0, 0);
            if (inRemoving && !name.isEmpty() && name != a.package) {
                also << name;
            }
        }
        for (const QString &pkg : std::as_const(also)) {
            if (isCore(pkg)) {
                Q_EMIT planReady(a.desktop, false, also,
                                 QStringLiteral("Desinstalar isso removeria partes essenciais do sistema (%1).").arg(pkg));
                return;
            }
        }
        Q_EMIT planReady(a.desktop, true, also, {});
    });
    p->start(QStringLiteral("dnf"), {QStringLiteral("remove"), QStringLiteral("--assumeno"), QStringLiteral("-q"), a.package});
}

void AppsModel::uninstall(int row)
{
    if (row < 0 || row >= m_visible.size()) {
        return;
    }
    const App a = m_apps.at(m_visible.at(row));
    const auto done = [this, a](bool ok, const QString &message) {
        setBusy(a.desktop, false);
        if (ok) {
            for (int i = 0; i < m_apps.size(); ++i) {
                if (m_apps.at(i).desktop == a.desktop) {
                    m_apps.removeAt(i);
                    break;
                }
            }
            rebuild();
            QProcess::startDetached(QStringLiteral("kbuildsycoca6"), {});
        }
        Q_EMIT uninstallFinished(a.name, ok, message);
    };

    if (a.kind == AppImage || a.kind == Shortcut) {
        // vai para a lixeira: dá para voltar atrás
        bool ok = QFile::moveToTrash(a.desktop);
        if (ok && a.kind == AppImage && QFile::exists(a.package)) {
            ok = QFile::moveToTrash(a.package);
        }
        done(ok, ok ? QString() : QStringLiteral("Não foi possível mover para a lixeira (sem permissão?)."));
        return;
    }

    QString program;
    QStringList args;
    if (a.kind == Rpm) {
        program = QStringLiteral("pkexec");
        args = {QStringLiteral("dnf"), QStringLiteral("remove"), QStringLiteral("-y"), QStringLiteral("-q"), a.package};
    } else if (a.kind == Flatpak) {
        program = QStringLiteral("flatpak");
        args = {QStringLiteral("uninstall"), QStringLiteral("-y"), QStringLiteral("--noninteractive"),
                a.flatpakInstallation == QLatin1String("user") ? QStringLiteral("--user") : QStringLiteral("--system"), a.package};
    } else {
        program = QStringLiteral("pkexec");
        args = {QStringLiteral("snap"), QStringLiteral("remove"), a.package};
    }
    setBusy(a.desktop, true);
    auto *p = process(this);
    connect(p, &QProcess::finished, this, [p, done](int code) {
        if (code == 126 || code == 127) { // pkexec: senha cancelada / não autorizado
            done(false, QStringLiteral("Desinstalação cancelada."));
            return;
        }
        const QString err = QString::fromUtf8(p->readAllStandardError()).trimmed();
        done(code == 0, code == 0 ? QString() : (err.isEmpty() ? QStringLiteral("Falhou (código %1).").arg(code) : err.section(QLatin1Char('\n'), -1)));
    });
    connect(p, &QProcess::errorOccurred, this, [done](QProcess::ProcessError error) {
        if (error == QProcess::FailedToStart) {
            done(false, QStringLiteral("Não foi possível executar o desinstalador."));
        }
    });
    p->start(program, args);
}
