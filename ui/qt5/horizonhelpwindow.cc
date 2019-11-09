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

HorizonHelpWindow::HorizonHelpWindow(QFile *helpFile, QWidget *parent) :
    QDialog(parent), helpFile(helpFile) {
    QDialogButtonBox *buttonBox;
    QTextEdit *helpText;
    QVBoxLayout *layout;

    setFixedSize(QSize(600, 400));
    setSizeGripEnabled(false);
    setWindowTitle("Horizon Help");

    helpText = new QTextEdit(this);
    helpText->setReadOnly(true);
    helpText->setHtml(helpFile->readAll().toStdString().c_str());

    buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok, this);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);

    layout = new QVBoxLayout();
    layout->addWidget(helpText);
    layout->addWidget(buttonBox);

    setLayout(layout);
}
