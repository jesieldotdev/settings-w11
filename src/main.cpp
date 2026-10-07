/*
 * settings-w11 — Configurações do KDE Plasma organizadas como as do Windows 11.
 *
 *   settings-w11                abre em Sistema
 *   settings-w11 kcm_pulseaudio abre direto num módulo (como o systemsettings)
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "mainwindow.h"

#include <KAboutData>
#include <KDBusService>

#include <QApplication>
#include <QCommandLineParser>

int main(int argc, char **argv)
{
    QApplication app(argc, argv);
    KAboutData about(QStringLiteral("settings-w11"), QStringLiteral("Configurações"), QStringLiteral("1.0"),
                     QStringLiteral("Configurações do KDE Plasma organizadas como as do Windows 11"), KAboutLicense::GPL_V2);
    about.setDesktopFileName(QStringLiteral("settings-w11"));
    KAboutData::setApplicationData(about);
    QApplication::setWindowIcon(QIcon::fromTheme(QStringLiteral("preferences-system")));

    QCommandLineParser parser;
    parser.addPositionalArgument(QStringLiteral("modulo"), QStringLiteral("Módulo a abrir (ex.: kcm_pulseaudio)"));
    about.setupCommandLine(&parser);
    parser.process(app);
    about.processCommandLine(&parser);

    MainWindow window;
    const QStringList args = parser.positionalArguments();
    if (!args.isEmpty()) {
        window.openModule(args.first());
    }

    // uma janela só: abrir de novo traz a mesma para a frente (e troca o módulo)
    KDBusService service(KDBusService::Unique);
    QObject::connect(&service, &KDBusService::activateRequested, &window, [&window](const QStringList &arguments, const QString &) {
        if (arguments.size() > 1) {
            window.openModule(arguments.at(1));
        }
        window.show();
        window.raise();
        window.activateWindow();
    });

    window.show();
    return app.exec();
}
