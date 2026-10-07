/*
 * Aplicativos instalados: os apps do menu, cada um ligado a quem o instalou
 * (pacote RPM, Flatpak, Snap, AppImage ou só um atalho), para listar e
 * desinstalar direto, sem passar pela loja.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#pragma once

#include <QAbstractListModel>
#include <QDateTime>
#include <QList>
#include <QVariantMap>

class QProcess;

class AppsModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(bool loading READ loading NOTIFY loadingChanged)
    Q_PROPERTY(QString filterText READ filterText WRITE setFilterText NOTIFY filterChanged)
    Q_PROPERTY(QString source READ source WRITE setSource NOTIFY filterChanged)   // "" = todas
    Q_PROPERTY(QString sortBy READ sortBy WRITE setSortBy NOTIFY filterChanged)   // name, name-desc, size, date
    Q_PROPERTY(int totalCount READ totalCount NOTIFY countsChanged)
    Q_PROPERTY(int count READ rowCount NOTIFY countsChanged)

public:
    enum Kind { Rpm, Flatpak, Snap, AppImage, Shortcut };
    Q_ENUM(Kind)

    enum Roles {
        NameRole = Qt::UserRole + 1,
        IconRole,
        KindRole,
        KindNameRole,
        PackageRole,
        VersionRole,
        PublisherRole,
        SizeRole,       // bytes (-1 = desconhecido)
        InstalledRole,  // QDateTime (inválido = desconhecido)
        DesktopRole,
        BusyRole,       // desinstalando agora
    };

    explicit AppsModel(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = {}) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    bool loading() const { return m_pending > 0; }
    QString filterText() const { return m_filterText; }
    void setFilterText(const QString &text);
    QString source() const { return m_source; }
    void setSource(const QString &source);
    QString sortBy() const { return m_sortBy; }
    void setSortBy(const QString &sortBy);
    int totalCount() const { return m_apps.size(); }

    Q_INVOKABLE void reload();
    Q_INVOKABLE void launch(int row);
    /** Pergunta o que sai junto (só RPM: simula no dnf). Responde em planReady. */
    Q_INVOKABLE void planUninstall(int row);
    Q_INVOKABLE void uninstall(int row);

Q_SIGNALS:
    void loadingChanged();
    void filterChanged();
    void countsChanged();
    /** ok=false: não dá para desinstalar (ex.: faz parte do sistema); message diz por quê. */
    void planReady(const QString &desktop, bool ok, const QStringList &alsoRemoved, const QString &message);
    void uninstallFinished(const QString &name, bool ok, const QString &message);

private:
    struct App {
        QString name;
        QString icon;
        Kind kind = Shortcut;
        QString package;   // nome do pacote / id do Flatpak / nome do Snap / caminho do AppImage
        QString version;
        QString publisher;
        qint64 size = -1;
        QDateTime installed;
        QString desktop;   // .desktop do menu
        QString flatpakInstallation; // user / system
        QString lookup;    // arquivo procurado no rpm (o do sistema, se o do menu for uma cópia local)
        bool busy = false;
    };

    void loadServices();
    void queryRpm();
    void queryFlatpak();
    void querySnap();
    void finishOne();
    void rebuild();
    App *findByDesktop(const QString &desktop);
    void setBusy(const QString &desktop, bool busy);
    static QString kindName(Kind kind);

    QList<App> m_apps;
    QList<int> m_visible; // índices de m_apps, filtrados e ordenados
    int m_pending = 0;
    QString m_filterText;
    QString m_source;
    QString m_sortBy = QStringLiteral("name");
};
