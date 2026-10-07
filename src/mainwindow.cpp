/*
 * settings-w11 — Configurações do KDE Plasma organizadas como as do Windows 11.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "mainwindow.h"

#include <KCModule>
#include <KCModuleLoader>
#include <KConfig>
#include <KConfigGroup>
#include <KPluginMetaData>
#include <KUser>

#include <QApplication>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QHBoxLayout>
#include <QIcon>
#include <QInputDialog>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QMessageBox>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QProcess>
#include <QQmlError>
#include <QQmlContext>
#include <QQuickItem>
#include <QQuickWindow>
#include <QQuickWidget>
#include <QPushButton>
#include <QRegularExpression>
#include <QScrollArea>
#include <QStyledItemDelegate>
#include <QSysInfo>
#include <QTimer>
#include <QToolButton>
#include <QUrl>
#include <QVBoxLayout>

#include <functional>

namespace
{
QColor alpha(QColor c, qreal a)
{
    c.setAlphaF(a);
    return c;
}

QString plain(QString s) // sem acentos e minúsculo, para a pesquisa
{
    s = s.normalized(QString::NormalizationForm_D).toLower();
    s.remove(QRegularExpression(QStringLiteral("[\\x{0300}-\\x{036f}]")));
    return s;
}

// Cartão do Windows 11: ícone, título, descrição e › (ou ↗ para programas à parte)
class Card : public QWidget
{
public:
    std::function<void()> onClick;

    Card(const Entry &entry, QWidget *parent)
        : QWidget(parent)
        , m_entry(entry)
    {
        setAttribute(Qt::WA_Hover);
        setCursor(Qt::PointingHandCursor);
        setMinimumHeight(m_entry.subtitle.isEmpty() ? 48 : 68);
        setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    }

    QSize sizeHint() const override
    {
        return QSize(600, minimumHeight());
    }

protected:
    void paintEvent(QPaintEvent *) override
    {
        QPainter p(this);
        p.setRenderHint(QPainter::Antialiasing);
        const QColor text = palette().color(QPalette::WindowText);
        p.setPen(QPen(alpha(text, 0.06), 1));
        p.setBrush(alpha(text, m_pressed ? 0.035 : (underMouse() ? 0.075 : 0.05)));
        p.drawRoundedRect(QRectF(rect()).adjusted(0.5, 0.5, -0.5, -0.5), 5, 5);

        QIcon::fromTheme(m_entry.icon, QIcon::fromTheme(QStringLiteral("preferences-system"))).paint(&p, QRect(20, (height() - 20) / 2, 20, 20));

        QFont titleFont = font();
        const QFontMetrics fm(titleFont);
        QFont small = font();
        small.setPointSizeF(font().pointSizeF() - 1);
        const QFontMetrics sfm(small);
        const int x = 56;
        const int textW = width() - x - 56;
        p.setPen(text);
        if (m_entry.subtitle.isEmpty()) {
            p.drawText(QRect(x, 0, textW, height()), Qt::AlignVCenter, fm.elidedText(m_entry.title, Qt::ElideRight, textW));
        } else {
            const int top = (height() - fm.height() - sfm.height()) / 2;
            p.drawText(QRect(x, top, textW, fm.height()), Qt::AlignVCenter, fm.elidedText(m_entry.title, Qt::ElideRight, textW));
            p.setFont(small);
            p.setPen(alpha(text, 0.65));
            p.drawText(QRect(x, top + fm.height(), textW, sfm.height()), Qt::AlignVCenter, sfm.elidedText(m_entry.subtitle, Qt::ElideRight, textW));
        }

        // › para entrar; ↗ para programa à parte
        p.setPen(QPen(alpha(text, 0.75), 1.3, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
        const QPointF c(width() - 28, height() / 2.0);
        if (!m_entry.command.isEmpty()) {
            p.drawLine(c + QPointF(-4, 4), c + QPointF(4, -4));
            p.drawPolyline(QPolygonF() << c + QPointF(-1, -4) << c + QPointF(4, -4) << c + QPointF(4, 1));
        } else {
            p.drawPolyline(QPolygonF() << c + QPointF(-2, -5) << c + QPointF(3, 0) << c + QPointF(-2, 5));
        }
    }

    void mousePressEvent(QMouseEvent *) override
    {
        m_pressed = true;
        update();
    }

    void mouseReleaseEvent(QMouseEvent *event) override
    {
        m_pressed = false;
        update();
        if (rect().contains(event->position().toPoint()) && onClick) {
            onClick();
        }
    }

private:
    Entry m_entry;
    bool m_pressed = false;
};

// Barra lateral: item selecionado com fundo arredondado e o tracinho azul à esquerda
class SidebarDelegate : public QStyledItemDelegate
{
public:
    using QStyledItemDelegate::QStyledItemDelegate;

    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override
    {
        return QSize(QStyledItemDelegate::sizeHint(option, index).width(), 38);
    }

    void paint(QPainter *p, const QStyleOptionViewItem &option, const QModelIndex &index) const override
    {
        p->save();
        p->setRenderHint(QPainter::Antialiasing);
        const QColor text = option.palette.color(QPalette::WindowText);
        const bool selected = option.state & QStyle::State_Selected;
        const bool hover = option.state & QStyle::State_MouseOver;
        const QRectF r = QRectF(option.rect).adjusted(4, 2, -4, -2);
        if (selected || hover) {
            p->setPen(Qt::NoPen);
            p->setBrush(alpha(text, selected ? 0.09 : 0.05));
            p->drawRoundedRect(r, 5, 5);
        }
        if (selected) {
            p->setBrush(option.palette.color(QPalette::Highlight));
            p->drawRoundedRect(QRectF(r.left(), r.center().y() - 8, 3, 16), 1.5, 1.5);
        }
        const QIcon icon = index.data(Qt::DecorationRole).value<QIcon>();
        icon.paint(p, QRect(int(r.left()) + 14, int(r.center().y()) - 9, 18, 18));
        p->setPen(text);
        p->drawText(QRectF(r.left() + 44, r.top(), r.width() - 48, r.height()), Qt::AlignVCenter, index.data().toString());
        p->restore();
    }
};

KPluginMetaData findModule(const QString &id)
{
    for (const auto &ns : {QStringLiteral("plasma/kcms/systemsettings"), QStringLiteral("plasma/kcms/systemsettings_qwidgets"),
                           QStringLiteral("plasma/kcms"), QStringLiteral("kcms")}) {
        const KPluginMetaData md = KPluginMetaData::findPluginById(ns, id);
        if (md.isValid()) {
            return md;
        }
    }
    return {};
}

QString wallpaperPath()
{
    // imagem de fundo atual do Plasma ([Containments][N][Wallpaper][<plugin>][General] Image=)
    KConfig config(QStringLiteral("plasma-org.kde.plasma.desktop-appletsrc"));
    const KConfigGroup containments = config.group(QStringLiteral("Containments"));
    for (const QString &group : containments.groupList()) {
        const KConfigGroup containment = containments.group(group);
        // o plugin ativo (imagem fixa ou apresentação de slides) guarda a imagem atual
        const QString plugin = containment.readEntry("wallpaperplugin", QString());
        if (plugin.isEmpty()) {
            continue;
        }
        const KConfigGroup general = containment.group(QStringLiteral("Wallpaper")).group(plugin).group(QStringLiteral("General"));
        QString image = general.readEntry("Image", QString());
        if (image.isEmpty()) {
            continue;
        }
        image = QUrl(image).isLocalFile() ? QUrl(image).toLocalFile() : image;
        if (QFileInfo(image).isDir()) { // pacote de papel de parede: pega a maior imagem
            QDir dir(image + QStringLiteral("/contents/images"));
            const auto files = dir.entryInfoList(QDir::Files, QDir::Size);
            image = files.isEmpty() ? QString() : files.first().absoluteFilePath();
        }
        if (QFileInfo::exists(image)) {
            return image;
        }
    }
    return {};
}

QString readSys(const QString &name)
{
    QFile f(QStringLiteral("/sys/class/dmi/id/") + name);
    return f.open(QIODevice::ReadOnly) ? QString::fromUtf8(f.readAll()).trimmed() : QString();
}
} // namespace

MainWindow::MainWindow(QWidget *parent)
    : QWidget(parent)
    , m_catalog(catalog())
{
    setWindowTitle(QStringLiteral("Configurações"));
    setWindowIcon(QIcon::fromTheme(QStringLiteral("preferences-system")));
    resize(1100, 740);
    setMinimumSize(760, 520);

    auto *layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    layout->addWidget(buildSidebar());

    // lado direito: título grande ("Sistema › Som") e a página
    auto *right = new QWidget(this);
    auto *rightLayout = new QVBoxLayout(right);
    rightLayout->setContentsMargins(28, 18, 28, 0);
    rightLayout->setSpacing(12);
    m_title = new QLabel(right);
    QFont titleFont = font();
    titleFont.setPointSizeF(font().pointSizeF() * 2.0);
    titleFont.setWeight(QFont::DemiBold);
    m_title->setFont(titleFont);
    m_title->setTextFormat(Qt::RichText);
    m_title->setTextInteractionFlags(Qt::LinksAccessibleByMouse);
    // clicar num nível anterior da trilha volta para ele
    connect(m_title, &QLabel::linkActivated, this, [this](const QString &link) {
        if (!leaveModule()) {
            return;
        }
        Route r;
        r.section = m_current.section;
        r.path = m_current.path.mid(0, link.toInt());
        showRoute(r);
    });
    rightLayout->addWidget(m_title);
    m_page = new QWidget(right);
    m_pageLayout = new QVBoxLayout(m_page);
    m_pageLayout->setContentsMargins(0, 0, 0, 0);
    rightLayout->addWidget(m_page, 1);
    layout->addWidget(right, 1);

    showRoute(Route{}, false);
}

QWidget *MainWindow::buildSidebar()
{
    auto *side = new QWidget(this);
    side->setFixedWidth(300);
    auto *col = new QVBoxLayout(side);
    col->setContentsMargins(12, 10, 8, 12);
    col->setSpacing(8);

    // ← voltar, como no canto da janela do Windows
    m_back = new QToolButton(side);
    m_back->setIcon(QIcon::fromTheme(QStringLiteral("go-previous")));
    m_back->setAutoRaise(true);
    m_back->setToolTip(QStringLiteral("Voltar"));
    connect(m_back, &QToolButton::clicked, this, &MainWindow::goBack);
    col->addWidget(m_back, 0, Qt::AlignLeft);

    // conta: foto, nome e "Conta local"
    KUser user;
    auto *account = new QWidget(side);
    auto *accountRow = new QHBoxLayout(account);
    accountRow->setContentsMargins(6, 0, 0, 6);
    accountRow->setSpacing(12);
    auto *avatar = new QLabel(account);
    QPixmap face(user.faceIconPath());
    if (face.isNull()) {
        face = QIcon::fromTheme(QStringLiteral("user-identity")).pixmap(64, 64);
    }
    const int size = 60;
    QPixmap round(QSize(size, size) * devicePixelRatioF());
    round.setDevicePixelRatio(devicePixelRatioF());
    round.fill(Qt::transparent);
    {
        QPainter p(&round);
        p.setRenderHint(QPainter::Antialiasing);
        QPainterPath clip;
        clip.addEllipse(0, 0, size, size);
        p.setClipPath(clip);
        p.drawPixmap(QRect(0, 0, size, size), face.scaled(QSize(size, size) * devicePixelRatioF(), Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation));
    }
    avatar->setPixmap(round);
    accountRow->addWidget(avatar);
    auto *names = new QVBoxLayout;
    names->setSpacing(0);
    auto *name = new QLabel(user.property(KUser::FullName).toString().isEmpty() ? user.loginName() : user.property(KUser::FullName).toString(), account);
    QFont bold = font();
    bold.setWeight(QFont::DemiBold);
    name->setFont(bold);
    auto *kind = new QLabel(QStringLiteral("Conta local"), account);
    kind->setEnabled(false);
    names->addStretch();
    names->addWidget(name);
    names->addWidget(kind);
    names->addStretch();
    accountRow->addLayout(names, 1);
    col->addWidget(account);

    m_search = new QLineEdit(side);
    m_search->setPlaceholderText(QStringLiteral("Localizar uma configuração"));
    m_search->setClearButtonEnabled(true);
    m_search->addAction(QIcon::fromTheme(QStringLiteral("search")), QLineEdit::TrailingPosition);
    m_search->setMinimumHeight(34);
    connect(m_search, &QLineEdit::textChanged, this, [this](const QString &text) {
        Route r;
        r.section = m_current.section;
        r.search = text.trimmed();
        showRoute(r, false);
    });
    col->addWidget(m_search);

    m_sections = new QListWidget(side);
    m_sections->setFrameShape(QFrame::NoFrame);
    m_sections->setItemDelegate(new SidebarDelegate(m_sections));
    m_sections->setMouseTracking(true);
    m_sections->viewport()->setAttribute(Qt::WA_Hover);
    QPalette pal = m_sections->palette();
    pal.setColor(QPalette::Base, Qt::transparent);
    m_sections->setPalette(pal);
    for (const Section &s : std::as_const(m_catalog)) {
        m_sections->addItem(new QListWidgetItem(QIcon::fromTheme(s.icon), s.title));
    }
    connect(m_sections, &QListWidget::itemClicked, this, [this](QListWidgetItem *item) {
        if (!leaveModule()) {
            m_sections->setCurrentRow(m_current.section);
            return;
        }
        m_search->blockSignals(true);
        m_search->clear();
        m_search->blockSignals(false);
        Route r;
        r.section = m_sections->row(item);
        showRoute(r);
    });
    col->addWidget(m_sections, 1);
    return side;
}

const Entry *MainWindow::entryAt(const Route &route) const
{
    const Entry *entry = nullptr;
    const QList<Entry> *list = &m_catalog.at(route.section).entries;
    for (int i : route.path) {
        entry = &list->at(i);
        list = &entry->children;
    }
    return entry;
}

QString MainWindow::breadcrumb(const Route &route) const
{
    if (!route.search.isEmpty()) {
        return QStringLiteral("Resultados da pesquisa");
    }
    QStringList parts{m_catalog.at(route.section).title};
    const QList<Entry> *list = &m_catalog.at(route.section).entries;
    for (int i : route.path) {
        parts << list->at(i).title;
        list = &list->at(i).children;
    }
    if (!route.kcm.isEmpty()) {
        parts << findModule(route.kcm).name();
    }
    // como no Windows: os níveis anteriores apagados e clicáveis, o atual em destaque
    const QString dim = palette().color(QPalette::PlaceholderText).name();
    QString html;
    for (int i = 0; i < parts.size(); ++i) {
        const bool last = i == parts.size() - 1;
        html += last ? parts[i].toHtmlEscaped()
                     : QStringLiteral("<a href='%1' style='color:%2; text-decoration:none'>%3</a><span style='color:%2'>&nbsp;&nbsp;›&nbsp;&nbsp;</span>")
                           .arg(QString::number(i), dim, parts[i].toHtmlEscaped());
    }
    return html;
}

void MainWindow::showRoute(const Route &route, bool push)
{
    if (push) {
        m_history.append(m_current);
    }
    m_current = route;
    m_back->setEnabled(!m_history.isEmpty());
    m_sections->setCurrentRow(route.section);
    m_title->setText(breadcrumb(route));

    // troca a página
    m_module = nullptr;
    while (QLayoutItem *item = m_pageLayout->takeAt(0)) {
        if (item->widget()) {
            item->widget()->deleteLater();
        }
        delete item;
    }
    QWidget *page = nullptr;
    const Entry *entry = entryAt(route);
    if (!route.search.isEmpty()) {
        page = searchPage(route.search);
    } else if (!route.kcm.isEmpty()) {
        Entry outside;
        outside.title = findModule(route.kcm).name();
        outside.kcm = route.kcm;
        page = modulePage(outside);
    } else if (entry && !entry->page.isEmpty()) {
        page = qmlPage(*entry);
    } else if (entry && !entry->kcm.isEmpty()) {
        page = modulePage(*entry);
    } else {
        page = sectionPage(route);
    }
    m_pageLayout->addWidget(page);
}

void MainWindow::goBack()
{
    if (m_history.isEmpty() || !leaveModule()) {
        return;
    }
    const Route previous = m_history.takeLast();
    m_search->blockSignals(true);
    m_search->setText(previous.search);
    m_search->blockSignals(false);
    showRoute(previous, false);
}

bool MainWindow::leaveModule()
{
    if (!m_module || !m_module->needsSave()) {
        return true;
    }
    const auto answer = QMessageBox::question(this, QStringLiteral("Alterações não aplicadas"),
                                              QStringLiteral("Há alterações que ainda não foram aplicadas. Aplicar agora?"),
                                              QMessageBox::Apply | QMessageBox::Discard | QMessageBox::Cancel, QMessageBox::Apply);
    if (answer == QMessageBox::Apply) {
        m_module->save();
    }
    return answer != QMessageBox::Cancel;
}

void MainWindow::activate(const Entry &entry, const Route &route)
{
    if (!entry.command.isEmpty()) {
        const QStringList args = QProcess::splitCommand(entry.command);
        QProcess::startDetached(args.first(), args.mid(1));
        return;
    }
    if (!entry.kcm.isEmpty() && !findModule(entry.kcm).isValid()) {
        QMessageBox::information(this, entry.title, QStringLiteral("Este módulo não está instalado neste sistema."));
        return;
    }
    showRoute(route);
}

QWidget *MainWindow::cardList(const QList<QPair<Entry, Route>> &cards)
{
    auto *scroll = new QScrollArea;
    scroll->setFrameShape(QFrame::NoFrame);
    scroll->setWidgetResizable(true);
    scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    auto *content = new QWidget;
    auto *col = new QVBoxLayout(content);
    col->setContentsMargins(0, 0, 8, 20);
    col->setSpacing(3);
    for (const auto &[entry, route] : cards) {
        auto *card = new Card(entry, content);
        const Entry e = entry;
        const Route r = route;
        card->onClick = [this, e, r]() {
            activate(e, r);
        };
        col->addWidget(card);
    }
    col->addStretch(1);
    scroll->setWidget(content);
    return scroll;
}

QWidget *MainWindow::sectionPage(const Route &route)
{
    const Entry *parent = entryAt(route);
    const QList<Entry> &entries = parent ? parent->children : m_catalog.at(route.section).entries;
    QList<QPair<Entry, Route>> cards;
    for (int i = 0; i < entries.size(); ++i) {
        // esconde o que não existe neste sistema (ex.: celular, Thunderbolt)
        if (!entries[i].kcm.isEmpty() && !findModule(entries[i].kcm).isValid()) {
            continue;
        }
        Route r = route;
        r.path.append(i);
        cards.append({entries[i], r});
    }
    QWidget *list = cardList(cards);
    if (route.section != 0 || !route.path.isEmpty()) {
        return list;
    }
    // Sistema: cabeçalho com o papel de parede, o nome do computador e "Renomear"
    auto *page = new QWidget;
    auto *col = new QVBoxLayout(page);
    col->setContentsMargins(0, 0, 0, 0);
    col->setSpacing(14);
    col->addWidget(systemHeader());
    col->addWidget(list, 1);
    return page;
}

QWidget *MainWindow::systemHeader()
{
    auto *header = new QWidget;
    auto *row = new QHBoxLayout(header);
    row->setContentsMargins(0, 0, 0, 0);
    row->setSpacing(16);

    auto *thumb = new QLabel(header);
    const QSize size(176, 99);
    QPixmap pix(size * devicePixelRatioF());
    pix.setDevicePixelRatio(devicePixelRatioF());
    pix.fill(Qt::transparent);
    {
        QPainter p(&pix);
        p.setRenderHint(QPainter::Antialiasing);
        QPainterPath clip;
        clip.addRoundedRect(QRectF(0, 0, size.width(), size.height()), 6, 6);
        p.setClipPath(clip);
        const QPixmap wall(wallpaperPath());
        if (!wall.isNull()) {
            p.drawPixmap(QRect(QPoint(0, 0), size), wall.scaled(size * devicePixelRatioF(), Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation));
        } else {
            p.fillRect(QRect(QPoint(0, 0), size), palette().color(QPalette::Highlight));
        }
    }
    thumb->setPixmap(pix);
    row->addWidget(thumb);

    auto *info = new QVBoxLayout;
    info->setSpacing(2);
    auto *host = new QLabel(QSysInfo::machineHostName(), header);
    QFont big = font();
    big.setPointSizeF(font().pointSizeF() * 1.3);
    big.setWeight(QFont::DemiBold);
    host->setFont(big);
    QString model = (readSys(QStringLiteral("sys_vendor")) + QLatin1Char(' ') + readSys(QStringLiteral("product_name"))).trimmed();
    if (model.isEmpty()) {
        model = QSysInfo::prettyProductName();
    }
    auto *modelLabel = new QLabel(model, header);
    auto *rename = new QPushButton(QStringLiteral("Renomear"), header);
    rename->setFlat(true);
    rename->setCursor(Qt::PointingHandCursor);
    rename->setStyleSheet(QStringLiteral("QPushButton { color: palette(highlight); text-align: left; padding: 0; border: none; }"));
    connect(rename, &QPushButton::clicked, this, [this, host]() {
        bool ok = false;
        const QString name = QInputDialog::getText(this, QStringLiteral("Renomear este PC"), QStringLiteral("Novo nome do computador:"),
                                                   QLineEdit::Normal, host->text(), &ok).trimmed();
        if (!ok || name.isEmpty() || name == host->text()) {
            return;
        }
        if (QProcess::execute(QStringLiteral("hostnamectl"), {QStringLiteral("set-hostname"), name}) == 0) {
            host->setText(name);
        } else {
            QMessageBox::warning(this, QStringLiteral("Renomear este PC"), QStringLiteral("Não foi possível trocar o nome do computador."));
        }
    });
    info->addStretch();
    info->addWidget(host);
    info->addWidget(modelLabel);
    info->addWidget(rename, 0, Qt::AlignLeft);
    info->addStretch();
    row->addLayout(info, 1);
    return header;
}

QWidget *MainWindow::searchPage(const QString &text)
{
    const QString needle = plain(text);
    QList<QPair<Entry, Route>> cards;
    std::function<void(const QList<Entry> &, Route)> walk = [&](const QList<Entry> &entries, Route base) {
        for (int i = 0; i < entries.size(); ++i) {
            Route r = base;
            r.path.append(i);
            const Entry &e = entries[i];
            const bool available = e.kcm.isEmpty() || findModule(e.kcm).isValid();
            if (available && plain(e.title + QLatin1Char(' ') + e.subtitle + QLatin1Char(' ') + e.keywords).contains(needle)
                && (e.children.isEmpty() || !e.kcm.isEmpty() || !e.command.isEmpty())) {
                Entry shown = e;
                shown.subtitle = m_catalog.at(r.section).title + QStringLiteral(" › ") + e.subtitle;
                cards.append({shown, r});
            }
            walk(e.children, r);
        }
    };
    for (int s = 0; s < m_catalog.size(); ++s) {
        Route r;
        r.section = s;
        walk(m_catalog.at(s).entries, r);
    }
    if (cards.isEmpty()) {
        auto *none = new QLabel(QStringLiteral("Nenhum resultado para \"%1\".").arg(text.toHtmlEscaped()));
        none->setAlignment(Qt::AlignTop | Qt::AlignLeft);
        none->setEnabled(false);
        return none;
    }
    return cardList(cards);
}

static void makeTransparent(QQuickWidget *view);

QWidget *MainWindow::modulePage(const Entry &entry)
{
    auto *page = new QWidget;
    auto *col = new QVBoxLayout(page);
    col->setContentsMargins(0, 0, 0, 12);
    col->setSpacing(8);

    const KPluginMetaData md = findModule(entry.kcm);
    KCModule *module = md.isValid() ? KCModuleLoader::loadModule(md, page) : nullptr;
    if (!module) {
        auto *error = new QLabel(QStringLiteral("Não foi possível abrir \"%1\".").arg(entry.title), page);
        error->setEnabled(false);
        col->addWidget(error);
        col->addStretch();
        return page;
    }
    m_module = module;
    module->load();
    // os módulos em QML pintam um painel opaco: tira, para aparecer a janela translúcida
    for (QQuickWidget *view : module->widget()->findChildren<QQuickWidget *>()) {
        makeTransparent(view);
    }
    col->addWidget(module->widget(), 1);

    // rodapé com Aplicar / Redefinir / Padrões quando o módulo usa
    const auto buttons = module->buttons();
    if (buttons & (KCModule::Apply | KCModule::Default)) {
        auto *footer = new QHBoxLayout;
        footer->setSpacing(8);
        auto *defaults = new QPushButton(QIcon::fromTheme(QStringLiteral("edit-undo")), QStringLiteral("Padrões"), page);
        defaults->setVisible(buttons & KCModule::Default);
        connect(defaults, &QPushButton::clicked, module, &KCModule::defaults);
        footer->addWidget(defaults);
        footer->addStretch();
        auto *reset = new QPushButton(QStringLiteral("Redefinir"), page);
        auto *apply = new QPushButton(QStringLiteral("Aplicar"), page);
        apply->setDefault(true);
        reset->setVisible(buttons & KCModule::Apply);
        apply->setVisible(buttons & KCModule::Apply);
        connect(reset, &QPushButton::clicked, module, &KCModule::load);
        connect(apply, &QPushButton::clicked, module, &KCModule::save);
        const auto update = [module, reset, apply, defaults]() {
            reset->setEnabled(module->needsSave());
            apply->setEnabled(module->needsSave());
            defaults->setEnabled(!module->representsDefaults());
        };
        connect(module, &KCModule::needsSaveChanged, page, update);
        connect(module, &KCModule::representsDefaultsChanged, page, update);
        update();
        footer->addWidget(reset);
        footer->addWidget(apply);
        col->addLayout(footer);
    }
    return page;
}

namespace
{
// O que as páginas QML podem pedir ao app (objeto "settings")
class Bridge : public QObject
{
    Q_OBJECT
public:
    explicit Bridge(MainWindow *window, QObject *parent)
        : QObject(parent)
        , m_window(window)
    {
    }

    Q_INVOKABLE void run(const QString &command)
    {
        const QStringList args = QProcess::splitCommand(command);
        if (!args.isEmpty()) {
            QProcess::startDetached(QStringLiteral("setsid"), QStringList{QStringLiteral("-f")} + args);
        }
    }

    Q_INVOKABLE void openRawModule(const QString &kcm)
    {
        // depois que o QML terminar de tratar o clique: a página vai ser trocada
        QMetaObject::invokeMethod(m_window, [w = m_window, kcm]() { w->openRawModule(kcm); }, Qt::QueuedConnection);
    }

private:
    MainWindow *m_window;
};
}

// Página QML por cima da janela translúcida, sem fundo próprio
static void stripBackgrounds(QQuickItem *item, const QColor &window)
{
    if (!item) {
        return;
    }
    // ApplicationItem (raiz dos módulos) e páginas do Kirigami têm fundo opaco
    if (item->metaObject()->indexOfProperty("color") >= 0 && item->metaObject()->indexOfProperty("pageStack") >= 0) {
        item->setProperty("color", QColor(Qt::transparent));
    }
    if (QString::fromLatin1(item->metaObject()->className()).contains(QLatin1String("Page"))
        && item->metaObject()->indexOfProperty("background") >= 0) {
        item->setProperty("background", QVariant::fromValue<QQuickItem *>(nullptr));
    }
    // faixas de fundo grandes na cor da janela (barra de título da página)
    if (QString::fromLatin1(item->metaObject()->className()) == QLatin1String("QQuickRectangle") && item->width() > 200 && item->height() > 30) {
        const QColor c = item->property("color").value<QColor>();
        if (c.alpha() == 255 && qAbs(c.lightness() - window.lightness()) < 24) {
            item->setProperty("color", QColor(Qt::transparent));
        }
    }
    for (QQuickItem *child : item->childItems()) {
        stripBackgrounds(child, window);
    }
}

static void makeTransparent(QQuickWidget *view)
{
    view->setAttribute(Qt::WA_AlwaysStackOnTop);
    view->setAttribute(Qt::WA_TranslucentBackground);
    view->setClearColor(Qt::transparent);
    // o conteúdo dos módulos chega aos poucos: limpa de novo quando a cena muda
    const QColor window = view->palette().color(QPalette::Window);
    auto strip = [view, window]() {
        stripBackgrounds(view->rootObject(), window);
    };
    strip();
    if (QQuickWindow *w = view->quickWindow()) {
        auto *timer = new QTimer(view);
        timer->setSingleShot(true);
        timer->setInterval(50);
        QObject::connect(timer, &QTimer::timeout, view, strip);
        QObject::connect(w, &QQuickWindow::sceneGraphInitialized, timer, qOverload<>(&QTimer::start));
        QObject::connect(w, &QQuickWindow::afterAnimating, timer, [timer]() {
            if (!timer->isActive()) {
                timer->start();
            }
        });
    }
}

QWidget *MainWindow::qmlPage(const Entry &entry)
{
    auto *view = new QQuickWidget;
    view->setResizeMode(QQuickWidget::SizeRootObjectToView);
    view->rootContext()->setContextProperty(QStringLiteral("settings"), new Bridge(this, view));
    view->setSource(QUrl(QStringLiteral("qrc:/pages/%1.qml").arg(entry.page)));
    makeTransparent(view);
    if (view->status() == QQuickWidget::Error) {
        for (const QQmlError &error : view->errors()) {
            qWarning() << error.toString();
        }
        // sem a página própria (ex.: falta o plasma-pa): cai no módulo do KDE
        if (!entry.kcm.isEmpty()) {
            delete view;
            return modulePage(entry);
        }
    }
    return view;
}

void MainWindow::openRawModule(const QString &kcm)
{
    if (!leaveModule()) {
        return;
    }
    Route r = m_current; // a trilha continua: Sistema › Som › Áudio
    r.kcm = kcm;
    showRoute(r);
}

bool MainWindow::openModule(const QString &kcm)
{
    std::function<bool(const QList<Entry> &, Route)> find = [&](const QList<Entry> &entries, Route base) {
        for (int i = 0; i < entries.size(); ++i) {
            Route r = base;
            r.path.append(i);
            if (entries[i].kcm == kcm) {
                showRoute(r, false);
                return true;
            }
            if (find(entries[i].children, r)) {
                return true;
            }
        }
        return false;
    };
    for (int s = 0; s < m_catalog.size(); ++s) {
        Route r;
        r.section = s;
        if (find(m_catalog.at(s).entries, r)) {
            return true;
        }
    }
    // fora do catálogo (ex.: módulo do KDE Connect): abre assim mesmo, com o nome dele
    if (findModule(kcm).isValid()) {
        Route r;
        r.kcm = kcm;
        showRoute(r, false);
        return true;
    }
    return false;
}

#include "mainwindow.moc"
