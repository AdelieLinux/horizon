/*
 * pkgdefaults.cc - Implementation of the UI.Packages.Choices page
 * horizon-qt5, the Qt 5 user interface for
 * Project Horizon
 *
 * Copyright (c) 2019 Adélie Linux and contributors.  All rights reserved.
 * This code is licensed under the AGPL 3.0 license, as noted in the
 * LICENSE-code file in the root directory of this repository.
 *
 * SPDX-License-Identifier: AGPL-3.0-only
 */

#include "pkgdefaults.hh"

#include <QButtonGroup>
#include <QHBoxLayout>
#include <QLabel>
#include <QRadioButton>
#include <QVBoxLayout>

PkgDefaultsPage::PkgDefaultsPage(QWidget *parent) : HorizonWizardPage(parent) {
    setTitle(tr("Software Choices"));
    loadWatermark("software");

    QVBoxLayout *mainLayout = new QVBoxLayout;

    QLabel *descLabel = new QLabel(tr(
        "You may customise the software used for certain functionality on this computer.\n\n"
        "Most users do not need to change any of the settings on this page.  "
        "If you're unsure of which options are best for you, review the Help system or keep the defaults."));
    descLabel->setWordWrap(true);
    mainLayout->addWidget(descLabel);
    mainLayout->addStretch();


    /******************** /bin/sh provider ********************/
    QButtonGroup *shellGroup = new QButtonGroup;
    QLabel *shellLabel = new QLabel(tr("Shell to use for /bin/sh:"));
    shellLabel->setWordWrap(true);

    dashShell = new QRadioButton("Dash");
    dashShell->setWhatsThis(tr("Use the lightweight Dash shell.  "
        "This is an Almquist-style shell, also used as /bin/sh on Debian-derived distributions.  "
        "Choose this option for faster boot times and full POSIX compatibility."));
    shellGroup->addButton(dashShell, HorizonWizard::Dash);

    bashShell = new QRadioButton("Bash");
    bashShell->setWhatsThis(tr("Use the Bash shell.  "
        "This shell is popular on GNU systems.  "
        "Choose this option for compatibility with non-portable scripts.  "
        "Note that by choosing this option, your system will no longer be able to conform to the POSIX standard."));
    shellGroup->addButton(bashShell, HorizonWizard::Bash);

    connect(shellGroup, static_cast<void (QButtonGroup:: *)(int)>(&QButtonGroup::buttonClicked),
            [=](int choice) {
        horizonWizard()->binsh = static_cast<HorizonWizard::BinShProvider>(choice);
    });

    QHBoxLayout *shellLayout = new QHBoxLayout;
    shellLayout->addWidget(dashShell);
    shellLayout->addWidget(bashShell);

    mainLayout->addWidget(shellLabel);
    mainLayout->addLayout(shellLayout);
    mainLayout->addStretch();


    /******************** /sbin/init provider ********************/
    QButtonGroup *initGroup = new QButtonGroup;
    QLabel *initLabel = new QLabel(tr("Init system (/sbin/init):"));
    initLabel->setWordWrap(true);

    s6Init = new QRadioButton("s6-linux-init");
    s6Init->setWhatsThis(tr("Use the lightweight, customisable s6-linux-init init system."));
    initGroup->addButton(s6Init, HorizonWizard::S6);

    sysvInit = new QRadioButton("SysV Init");
    sysvInit->setWhatsThis(tr("Use the traditional sysvinit init system."));
    initGroup->addButton(sysvInit, HorizonWizard::SysVInit);

    connect(initGroup, static_cast<void (QButtonGroup:: *)(int)>(&QButtonGroup::buttonClicked),
            [=](int choice) {
        horizonWizard()->sbininit = static_cast<HorizonWizard::InitSystem>(choice);
    });

    QHBoxLayout *initLayout = new QHBoxLayout;
    initLayout->addWidget(s6Init);
    initLayout->addWidget(sysvInit);

    mainLayout->addWidget(initLabel);
    mainLayout->addLayout(initLayout);
    mainLayout->addStretch();


    /******************** device event handler ********************/
    QButtonGroup *udevGroup = new QButtonGroup;
    QLabel *udevLabel = new QLabel(tr("uevent management daemon:"));
    udevLabel->setWordWrap(true);

    eudev = new QRadioButton("eudev");
    eudev->setWhatsThis(tr("Use the traditional, UDev-compatible eudev system.  "
        "It is highly recommended that you use eudev unless you know it is inappropriate for your use case."));
    udevGroup->addButton(eudev, true);

    mdevd = new QRadioButton("mdevd");
    mdevd->setWhatsThis(tr("Use the minimalist, lightweight mdevd system.  "
        "This is the skarnet fork of mdevd.  "
        "Choosing this option on a desktop system will require manual intervention."));
    udevGroup->addButton(mdevd, false);

    connect(udevGroup, static_cast<void (QButtonGroup:: *)(int)>(&QButtonGroup::buttonClicked),
            [=](int choice) {
        horizonWizard()->eudev = static_cast<bool>(choice);
    });

    QHBoxLayout *udevLayout = new QHBoxLayout;
    udevLayout->addWidget(eudev);
    udevLayout->addWidget(mdevd);

    mainLayout->addWidget(udevLabel);
    mainLayout->addLayout(udevLayout);
    mainLayout->addStretch();

    setLayout(mainLayout);
}

void PkgDefaultsPage::initializePage() {
    switch(horizonWizard()->binsh) {
    case HorizonWizard::Dash:
        dashShell->setChecked(true);
        bashShell->setChecked(false);
        break;
    case HorizonWizard::Bash:
        dashShell->setChecked(false);
        bashShell->setChecked(true);
        break;
    }

    switch(horizonWizard()->sbininit) {
    case HorizonWizard::S6:
        s6Init->setChecked(true);
        sysvInit->setChecked(false);
        break;
    case HorizonWizard::SysVInit:
        s6Init->setChecked(false);
        sysvInit->setChecked(true);
        break;
    }

    if(horizonWizard()->eudev) {
        eudev->setChecked(true);
        mdevd->setChecked(false);
    } else {
        eudev->setChecked(false);
        mdevd->setChecked(true);
    }
}
