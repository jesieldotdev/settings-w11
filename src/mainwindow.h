/*
 * settings-w11 — Configurações do KDE Plasma organizadas como as do Windows 11.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#pragma once

#include "catalog.h"

#include <QWidget>

class KCModule;
class QLabel;
class QLineEdit;
class QListWidget;
class QPushButton;
class QScrollArea;
class QStackedWidget;
class QToolButton;
class QVBoxLayout;

class MainWindow : public QWidget
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);

    /** Abre direto um módulo pelo id (ex.: kcm_pulseaudio), como o systemsettings faz. */
    bool openModule(const QString &kcm);

    /** Abre o módulo do KDE de verdade, mesmo quando há página própria para ele. */
    void openRawModule(const QString &kcm);

private:
    // Onde se está: seção + caminho de cartões dentro dela (grupos/módulo)
    struct Route {
        int section = 0;
        QList<int> path;
        QString search;
        QString kcm; // módulo fora do catálogo, pedido por outro programa
    };

    QWidget *buildSidebar();
    void showRoute(const Route &route, bool push = true);
    void goBack();
    const Entry *entryAt(const Route &route) const;
    QString breadcrumb(const Route &route) const;

    QWidget *sectionPage(const Route &route);
    QWidget *systemHeader();
    QWidget *searchPage(const QString &text);
    QWidget *modulePage(const Entry &entry);
    QWidget *qmlPage(const Entry &entry);
    QWidget *cardList(const QList<QPair<Entry, Route>> &cards);
    void activate(const Entry &entry, const Route &route);
    bool leaveModule(); // pergunta sobre alterações não aplicadas

    QList<Section> m_catalog;
    QList<Route> m_history;
    Route m_current;

    QListWidget *m_sections = nullptr;
    QLineEdit *m_search = nullptr;
    QToolButton *m_back = nullptr;
    QLabel *m_title = nullptr;
    QWidget *m_page = nullptr;
    QVBoxLayout *m_pageLayout = nullptr;
    KCModule *m_module = nullptr;
};
