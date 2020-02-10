/*
 * errorpage.cc - Implementation of the UI.Perform.Error page
 * horizon-run-qt5, the Qt 5 executor user interface for
 * Project Horizon
 *
 * Copyright (c) 2019 Adélie Linux and contributors.  All rights reserved.
 * This code is licensed under the AGPL 3.0 license, as noted in the
 * LICENSE-code file in the root directory of this repository.
 *
 * SPDX-License-Identifier: AGPL-3.0-only
 */

#include "errorpage.hh"

#include <QFileDialog>
#include <QHBoxLayout>
#include <QLabel>
#include <QMessageBox>
#include <QProcess>
#include <QPushButton>
#include <QVBoxLayout>

#include "executorwizard.hh"
#include "executepage.hh"
#include "../horizonhelpwindow.hh"

ErrorPage::ErrorPage(QWidget *parent) : HorizonWizardPage(parent) {
    setTitle(tr("Installation Couldn't Finish"));
    loadWatermark("intro");
    setFinalPage(true);

    descLabel = new QLabel(tr("I am Error."));
    descLabel->setWordWrap(true);
    deetLabel = new QLabel;
    deetLabel->setWordWrap(true);

    QPushButton *viewLog = new QPushButton(tr("&View Log"));
    viewLog->setWhatsThis(tr("Opens a log viewer, in which you can examine the contents of the installation log file."));
    connect(viewLog, &QPushButton::clicked, [=]{
        QFile log("/var/log/horizon/executor.log");
        log.open(QFile::ReadOnly);
        HorizonHelpWindow window(&log, this, true);
        window.exec();
    });
    QPushButton *saveData = new QPushButton(tr("&Save Script/Log..."));
    saveData->setWhatsThis(tr("Allows you to save the HorizonScript and installation log files to removable media."));
    connect(saveData, &QPushButton::clicked, [=]{
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
    QPushButton *popAnXterm = new QPushButton(tr("Open &Terminal"));
    popAnXterm->setWhatsThis(tr("Opens a terminal, to allow you to run commands from the installation environment."));
    connect(popAnXterm, &QPushButton::clicked, [=]{
        QProcess p;
        p.execute("xterm", {"-fa", "Liberation Mono", "-fs", "12"});
    });

    QHBoxLayout *buttonLayout = new QHBoxLayout;
    buttonLayout->addWidget(viewLog);
    buttonLayout->addWidget(saveData);
    buttonLayout->addWidget(popAnXterm);

    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(descLabel);
    layout->addStretch();
    layout->addWidget(deetLabel);
    layout->addLayout(buttonLayout);

    setLayout(layout);
}

void ErrorPage::initializePage() {
    ExecutePage *epage = dynamic_cast<ExecutePage *>(wizard()->page(ExecutorWizard::Page_Execute));
    switch(epage->currentPhase()) {
    case ExecutePage::Prepare:
        descLabel->setText(tr(
            "An issue occurred while preparing this computer for installation.\n\n"
            "This almost always indicates a corrupted installation medium, or a hardware fault.\n\n"
            "Try creating a new installation medium.  If that doesn't work, you may need to have your computer serviced."));
        deetLabel->setText(tr(
            "Details: A failure during the Prepare phase means the Executor process failed to start, "
            "crashed during initialisation, or encountered an early parsing failure in the HorizonScript."));
        break;
    case ExecutePage::Validate:
        descLabel->setText(tr(
            "An issue occurred while validating the installation script."));
        deetLabel->setText(tr(
            "Details: If you used the System Installation Wizard, you have encountered a bug in the wizard.  "
            "Please choose \"Save Script/Log...\" and follow the instructions at https://horizon.adelielinux.org/bug to report this issue.\n\n"
            "If you wrote the script yourself, you have made a syntax error."));
        break;
    case ExecutePage::Disk:
        descLabel->setText(tr(
            "An issue occurred while manipulating this computer's hard disk(s).\n\n"
            "Ensure you have selected the correct hard disk drive."));
        deetLabel->setText(tr(
            "Details: This may indicate a disk is read-only, an issue with the disk's controller or cabling, or a failing disk.\n\n"
            "If the correct disk is selected, check the cabling from the drive to the controller, and the controller to the computer.\n\n"
            "Otherwise, ensure the disk is not physically damaged.\n\n"
            "Review the log file for more information."));
        break;
    case ExecutePage::Net:
        descLabel->setText(tr(
            "An issue occurred while configuring this computer's network connection(s).\n\n"
            "Ensure your networking equipment is powered on, including any modems, routers, or switches.\n\n"
            "Also ensure that all cabling is firmly connected."));
        deetLabel->setText(tr(
            "Details: The system could not correctly configure the network based on the information provided.  "
            "This may be due to a configuration error, a cabling/equipment issue, or your network adaptor may require firmware.  "
            "Review the log file for more information."));
        break;
    case ExecutePage::Pkg:
        descLabel->setText(tr("An issue occurred while installing packages."));
        deetLabel->setText(tr(
            "Details: This failure typically indicates an issue connecting to the network.  "
            "Ensure your network configuration is correct.\n\n"
            "You may be attempting to install a version or edition of Adélie Linux that your mirror does not provide.\n\n"
            "Rarely, this may indicate an issue with your installation media, "
            "or that software you have selected for installation is no longer available."));
        break;
    case ExecutePage::PreMeta:
#if __cplusplus >= 201703L
        [[ fallthrough ]];
#endif
    case ExecutePage::PostMeta:
        descLabel->setText(tr("An issue occurred while configuring this computer."));
        deetLabel->setText(tr("Details: This typically means writing a configuration file has failed.  "
                              "Review the log file for more information."));
        break;
    }
}
