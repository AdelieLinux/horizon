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

    WaitDialog d;
    d.show();
    app.setOverrideCursor(Qt::WaitCursor);
    app.processEvents(QEventLoop::AllEvents, 300);

    app.setWindowIcon(QIcon(":/horizon-256.png"));

    HorizonWizard wizard;
    d.hide();
    app.restoreOverrideCursor();
    wizard.show();

    return app.exec();
}
