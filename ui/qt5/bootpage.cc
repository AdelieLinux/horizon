/*
 * bootpage.cc - Implementation of the UI.Boot page
 * horizon-qt5, the Qt 5 user interface for
 * Project Horizon
 *
 * Copyright (c) 2019 Adélie Linux and contributors.  All rights reserved.
 * This code is licensed under the AGPL 3.0 license, as noted in the
 * LICENSE-code file in the root directory of this repository.
 *
 * SPDX-License-Identifier: AGPL-3.0-only
 */

#include "bootpage.hh"

#include <QButtonGroup>
#include <QLabel>
#include <QVBoxLayout>

#include <Section.h>

BootPage::BootPage(QWidget *parent) : HorizonWizardPage(parent) {
    setTitle(tr("Startup Configuration"));
    loadWatermark("software");

    QLabel *descLabel = new QLabel(tr(
        "<p>By default, Adélie Linux will install a bootloader to your hard disk.  "
        "If you will be using multiple operating systems on this computer (not common), you may need to select \"Do not install a boot loader\".</p>"
        "<p><strong>Most users should choose the default.</strong></p>"
        "<p>Do you want to install a bootloader?</p>"));
    descLabel->setTextFormat(Qt::RichText);
    descLabel->setWordWrap(true);

    yesGrub = new QRadioButton(tr("&Install a boot loader."));
    yesGrub->setWhatsThis(tr("This option will install a bootloader to your hard disk, replacing any other bootloader if present."));
    noGrub = new QRadioButton(tr("&Do not install a boot loader."));
    noGrub->setWhatsThis(tr("This option will not install a bootloader to your hard disk.  You may be required to configure a bootloader yourself before your computer will start up properly."));
    QButtonGroup *grubChoices = new QButtonGroup;
    grubChoices->addButton(yesGrub, true);
    grubChoices->addButton(noGrub, false);
    connect(grubChoices, static_cast<void (QButtonGroup:: *)(int)>(&QButtonGroup::buttonClicked),
            [=](int choice) {
        horizonWizard()->grub = static_cast<bool>(choice);
    });

    Section *kernDisclosure = new Section("Kernel options");

    /* We set the default option, chosen during wizard initialisation,
     * as checked. */
    easyKernel = new QRadioButton(tr("Use the &stable kernel."),
                                                kernDisclosure);
    easyKernel->setWhatsThis(tr("This option will install the LTS version of the Linux kernel, which is well supported and thoroughly tested.  However, it may not support the newest hardware options."));
    mlKernel = new QRadioButton(tr("Use the &mainline kernel. (Potentially unstable)"),
                                              kernDisclosure);
    mlKernel->setWhatsThis(tr("This option will install the latest version of the Linux kernel, which has not been as thoroughly tested as the stable kernel.  Use this option if you have hardware that requires a newer kernel version than LTS."));
    QButtonGroup *kernelChoices = new QButtonGroup;
    kernelChoices->addButton(easyKernel);
    kernelChoices->addButton(mlKernel);
    connect(kernelChoices, static_cast<void(QButtonGroup:: *)(QAbstractButton *)>(&QButtonGroup::buttonClicked),
            [=](QAbstractButton *button) {
        if(button == easyKernel) {
            horizonWizard()->kernel = "easy-kernel";
        } else if(button == mlKernel) {
            horizonWizard()->kernel = "mainline-kernel";
        }
    });

    QVBoxLayout *kernelLayout = new QVBoxLayout;
    kernelLayout->addWidget(easyKernel);
    kernelLayout->addWidget(mlKernel);

    kernDisclosure->setContentLayout(*kernelLayout);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(descLabel);
    mainLayout->addSpacing(25);
    mainLayout->addWidget(yesGrub);
    mainLayout->addWidget(noGrub);
    mainLayout->addStretch();
    mainLayout->addWidget(kernDisclosure);
    setLayout(mainLayout);
}

void BootPage::initializePage() {
    yesGrub->click();

    if(horizonWizard()->kernel == "easy-kernel") {
        easyKernel->setChecked(true);
    } else if(horizonWizard()->kernel == "mainline-kernel") {
        mlKernel->setChecked(true);
    } else {
        /* ?! */
    }
}
