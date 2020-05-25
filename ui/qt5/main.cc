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
#include <QCommandLineParser>
#include <QDialog>
#include <QIcon>
#include <QLabel>
#include <QLibraryInfo>
#include <QTranslator>
#include <QVBoxLayout>

#include "horizonwizard.hh"

class WaitDialog : public QDialog {
public:
    WaitDialog(QWidget *parent = nullptr);
    void accept() override;
    void reject() override;
};

WaitDialog::WaitDialog(QWidget *parent) : QDialog(parent,
                                                  Qt::FramelessWindowHint) {
   setWindowTitle(tr("Loading..."));
   QVBoxLayout *layout = new QVBoxLayout;
   QLabel *descLabel = new QLabel(tr("System Installation is loading.\n\n"
                                     "Please wait a moment."));
   descLabel->setAlignment(Qt::AlignCenter);
   layout->addWidget(descLabel);
   setLayout(layout);
}

/* Prevents the user from interacting in any way. */
void WaitDialog::accept() { return; }
void WaitDialog::reject() { return; }

int main(int argc, char *argv[]) {
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QApplication app(argc, argv);
    app.setOrganizationName("Adélie Linux");
    app.setApplicationName("Horizon Qt UI");
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

    WaitDialog d;
    d.show();
    app.setOverrideCursor(Qt::WaitCursor);
    app.processEvents(QEventLoop::AllEvents, 1000);

    app.setWindowIcon(QIcon(":/horizon-256.png"));

    QCommandLineParser parser;
    parser.setApplicationDescription(app.tr("Guides the user through creation of a HorizonScript."));
    QCommandLineOption help = parser.addHelpOption();
    QCommandLineOption version = parser.addVersionOption();

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

    HorizonWizard wizard;
    d.hide();
    app.restoreOverrideCursor();
    wizard.show();

    return app.exec();
}
