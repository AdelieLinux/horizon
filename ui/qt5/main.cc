/*
 * main.cc - Implementation of the main routine
 * horizon-qt5, the Qt 5 user interface for
 * Project Horizon
 *
 * Copyright (c) 2019 Adélie Linux and contributors.  All rights reserved.
 * This code is licensed under the AGPL 3.0 license, as noted in the
 * LICENSE-code file in the root directory of this repository.
 *
 * SPDX-License-Identifier: AGPL-3.0-only
 */

#include <QApplication>
#include <QIcon>
#include <QLibraryInfo>
#include <QTranslator>

#include "horizonwizard.hh"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

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

    HorizonWizard wizard;
    wizard.show();

    return app.exec();
}
