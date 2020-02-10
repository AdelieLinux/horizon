/*
 * main.cc - Implementation of the UI.Perform interface
 * horizon-run-qt5, the Qt 5 executor user interface for
 * Project Horizon
 *
 * Copyright (c) 2019 Adélie Linux and contributors.  All rights reserved.
 * This code is licensed under the AGPL 3.0 license, as noted in the
 * LICENSE-code file in the root directory of this repository.
 *
 * SPDX-License-Identifier: AGPL-3.0-only
 */

#include <QApplication>
#include <QCommandLineParser>
#include <QIcon>
#include <QLibraryInfo>
#include <QTranslator>

#include "executorwizard.hh"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    app.setOrganizationName("Adélie Linux");
    app.setApplicationName("Horizon Runner GUI");
    app.setApplicationVersion(VERSTR);

    QString translatorFileName = QLatin1String("qt_");
    translatorFileName += QLocale::system().name();
    QTranslator *translator = new QTranslator(&app);
    if(translator->load(translatorFileName,
                        QLibraryInfo::location(
                                QLibraryInfo::TranslationsPath
                        ))) {
        app.installTranslator(translator);
    }

    app.setWindowIcon(QIcon(":/horizon-256.png"));

    QCommandLineParser parser;
    parser.setApplicationDescription(app.tr("Executes a HorizonScript with graphical progress indication."));
    QCommandLineOption help = parser.addHelpOption();
    QCommandLineOption version = parser.addVersionOption();
    QCommandLineOption automatic("automatic", app.tr("Runs the Executor in Automatic mode.  After installation, the system will be restarted automatically."));
    parser.addOption(automatic);

    if(!parser.parse(app.arguments())) {
        parser.showHelp(1);
        return 1;
    }

    if(parser.isSet(help)) {
        parser.showHelp();
        return 0;
    }

    if(parser.isSet(version)) {
        parser.showVersion();
        return 0;
    }

    ExecutorWizard wizard(nullptr, parser.isSet(automatic));
    wizard.show();

    return app.exec();
}
