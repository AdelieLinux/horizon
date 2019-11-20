/*
 * pkgsimple.cc - Implementation of the UI.Packages.SimpleSel page
 * horizon-qt5, the Qt 5 user interface for
 * Project Horizon
 *
 * Copyright (c) 2019 Adélie Linux and contributors.  All rights reserved.
 * This code is licensed under the AGPL 3.0 license, as noted in the
 * LICENSE-code file in the root directory of this repository.
 *
 * SPDX-License-Identifier: AGPL-3.0-only
 */

#include "pkgsimple.hh"

#include <QButtonGroup>
#include <QGridLayout>
#include <QLabel>
#include <QRadioButton>
#include <QVBoxLayout>

PkgSimplePage::PkgSimplePage(QWidget *parent) : HorizonWizardPage(parent) {
    setTitle(tr("Software Selection"));
    loadWatermark("software");

    QLabel *descLabel = new QLabel(tr(
        "Select the software you want to install on this computer.\n\n"
        "You can install and uninstall more software at any time using the Package Manager.  For more information, see the User's Handbook in Online Help."));
    descLabel->setWordWrap(true);

    QRadioButton *standardButton, *mobileButton, *compactButton, *textButton,
                 *customButton;
    QLabel *standardLabel, *mobileLabel, *compactLabel, *textLabel, *customLabel;
    standardButton = new QRadioButton(tr("&Standard"));
    standardButton->setIcon(QIcon::fromTheme("preferences-desktop-theme"));
    standardButton->setIconSize(QSize(32, 32));
    standardLabel = new QLabel(tr("Includes a full KDE desktop environment, including Web browser, email client, media player, and office suite."));
    standardLabel->setBuddy(standardButton);
    standardLabel->setWordWrap(true);

    mobileButton = new QRadioButton(tr("&Mobile"));
    mobileButton->setIcon(QIcon::fromTheme("battery"));
    mobileButton->setIconSize(QSize(32, 32));
    mobileLabel = new QLabel(tr("Includes the Standard software and additional utilities for notebook and tablet computers."));
    mobileLabel->setBuddy(mobileButton);
    mobileLabel->setWordWrap(true);

    compactButton = new QRadioButton(tr("&Compact"));
    compactButton->setIcon(QIcon::fromTheme("preferences-ubuntu-panel"));
    compactButton->setIconSize(QSize(32, 32));
    compactLabel = new QLabel(tr("Includes a lightweight LXQt desktop environment and a text editor."));
    compactLabel->setBuddy(compactButton);
    compactLabel->setWordWrap(true);

    textButton = new QRadioButton(tr("&Text-Only"));
    textButton->setIcon(QIcon::fromTheme("utilities-terminal"));
    textButton->setIconSize(QSize(32, 32));
    textLabel = new QLabel(tr("Includes support for text-mode only.  Select this option on servers, or computers with very limited resources."));
    textLabel->setBuddy(textButton);
    textLabel->setWordWrap(true);

    customButton = new QRadioButton(tr("C&ustom"));
    customButton->setIcon(QIcon::fromTheme("preferences-activities"));
    customButton->setIconSize(QSize(32, 32));
    customLabel = new QLabel(tr("Customise the packages installed on your computer."));
    customLabel->setBuddy(customButton);
    customLabel->setWordWrap(true);

    QButtonGroup *group = new QButtonGroup(this);
    group->addButton(standardButton, HorizonWizard::Standard);
    group->addButton(mobileButton, HorizonWizard::Mobile);
    group->addButton(compactButton, HorizonWizard::Compact);
    group->addButton(textButton, HorizonWizard::TextOnly);
    group->addButton(customButton, HorizonWizard::Custom);
    connect(group, static_cast<void (QButtonGroup:: *)(int)>(&QButtonGroup::buttonClicked),
            [=](int id) {
        horizonWizard()->pkgtype = static_cast<HorizonWizard::PackageType>(id);
    });

    QGridLayout *buttonLayout = new QGridLayout;
    buttonLayout->addWidget(standardButton, 0, 0);
    buttonLayout->addWidget(standardLabel, 0, 1);
    buttonLayout->addWidget(mobileButton, 1, 0);
    buttonLayout->addWidget(mobileLabel, 1, 1);
    buttonLayout->addWidget(compactButton, 2, 0);
    buttonLayout->addWidget(compactLabel, 2, 1);
    buttonLayout->addWidget(textButton, 3, 0);
    buttonLayout->addWidget(textLabel, 3, 1);
    buttonLayout->addWidget(customButton, 4, 0);
    buttonLayout->addWidget(customLabel, 4, 1);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(descLabel);
    mainLayout->addStretch();
    mainLayout->addLayout(buttonLayout);
    setLayout(mainLayout);
}

int PkgSimplePage::nextId() const {
    if(horizonWizard()->pkgtype == HorizonWizard::Custom)
        return HorizonWizard::Page_PkgCustom;

    return HorizonWizard::Page_Boot;
}
