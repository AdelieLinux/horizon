/*
 * finishedpage.cc - Implementation of the UI.Finish page
 * horizon-run-qt5, the Qt 5 executor user interface for
 * Project Horizon
 *
 * Copyright (c) 2019 Adélie Linux and contributors.  All rights reserved.
 * This code is licensed under the AGPL 3.0 license, as noted in the
 * LICENSE-code file in the root directory of this repository.
 *
 * SPDX-License-Identifier: AGPL-3.0-only
 */

#include "finishedpage.hh"

#include <QFileDialog>
#include <QLabel>
#include <QMessageBox>
#include <QPushButton>
#include <QVBoxLayout>

FinishedPage::FinishedPage(QWidget *parent) : HorizonWizardPage(parent) {
    setTitle(tr("Adélie Linux Successfully Installed"));
    loadWatermark("finish");

    QLabel *descLabel = new QLabel(tr(
        "Congratulations!\n\n"
        "Adélie Linux has been successfully installed on this computer.  "
        "Be sure to read our Handbooks to learn how to get the most out of your new operating environment.\n\n"
        "Remember to eject your installation media (CD, DVD, USB drive).\n\n"
        "We hope you enjoy using Adélie.\n\nChoose \"Finish\" to restart your computer."));
    descLabel->setWordWrap(true);

    QPushButton *owo = new QPushButton(tr("&Save Installation Data"));
    owo->setWhatsThis(tr("Saves the HorizonScript and log files associated with this installation."));
    owo->setAutoDefault(false);
    owo->setDefault(false);
    connect(owo, &QPushButton::clicked, [=]{
        bool success = true;

        QString dir = QFileDialog::getExistingDirectory(this, tr("Choose Directory"), "/target/root");
        if(dir.size() > 0) {
            success = QFile::copy("/etc/horizon/installfile", dir + QDir::separator() + "installfile");
            if(success) success = QFile::copy("/var/log/horizon/executor.log", dir + QDir::separator() + "install.log");
        }

        if(!success) {
            QMessageBox::critical(this, tr("Could Not Save Installation Data"), tr("Unable to save installation data to %1.").arg(dir));
        }
    });

    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(descLabel);
    layout->addStretch();
    layout->addWidget(owo, 0, Qt::AlignCenter);

    setLayout(layout);
}
