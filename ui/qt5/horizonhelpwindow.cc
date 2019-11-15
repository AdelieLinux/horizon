/*
 * horizonhelpwindow.cc - Implementation of the Help window
 * horizon-qt5, the Qt 5 user interface for
 * Project Horizon
 *
 * Copyright (c) 2019 Adélie Linux and contributors.  All rights reserved.
 * This code is licensed under the AGPL 3.0 license, as noted in the
 * LICENSE-code file in the root directory of this repository.
 *
 * SPDX-License-Identifier: AGPL-3.0-only
 */

#include "horizonhelpwindow.hh"

#include <QDialogButtonBox>
#include <QTextEdit>
#include <QVBoxLayout>

HorizonHelpWindow::HorizonHelpWindow(QFile *helpFile, QWidget *parent, bool log) :
    QDialog(parent), helpFile(helpFile) {
    QDialogButtonBox *buttonBox;
    QTextEdit *helpText;
    QVBoxLayout *layout;

    setFixedSize(QSize(600, 400));
    setSizeGripEnabled(false);

    helpText = new QTextEdit(this);
    helpText->setReadOnly(true);
    if(log) {
        setWindowTitle(helpFile->fileName());
        helpText->setText(helpFile->readAll().toStdString().c_str());
    } else {
        setWindowTitle("Horizon Help");
        helpText->setHtml(helpFile->readAll().toStdString().c_str());
    }
    helpText->setWhatsThis(tr("This window contains information about the current page of Adélie Linux System Installation."));

    buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok, this);
    buttonBox->setWhatsThis(tr("Choose OK to close this window and return to Adélie Linux System Installation."));
    connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);

    layout = new QVBoxLayout();
    layout->addWidget(helpText);
    layout->addWidget(buttonBox);

    setLayout(layout);
}
