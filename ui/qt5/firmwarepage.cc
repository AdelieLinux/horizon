/*
 * firmwarepage.cc - Implementation of the UI.Firmware page
 * horizon-qt5, the Qt 5 user interface for
 * Project Horizon
 *
 * Copyright (c) 2019 Adélie Linux and contributors.  All rights reserved.
 * This code is licensed under the AGPL 3.0 license, as noted in the
 * LICENSE-code file in the root directory of this repository.
 *
 * SPDX-License-Identifier: AGPL-3.0-only
 */

#include "firmwarepage.hh"

#include <QLabel>
#include <QVariant>
#include <QVBoxLayout>

FirmwarePage::FirmwarePage(QWidget *parent) : HorizonWizardPage(parent) {
    setTitle(tr("Load Firmware"));
    loadWatermark("intro");

    QLabel *descLabel = new QLabel(tr(
        "Your computer may use hardware drivers which require proprietary, closed-source firmware in order to provide functionality.\n\n"
        "Most Wi-Fi network adaptors and 3D graphics cards require proprietary firmware.  "
        "However, proprietary firmware cannot be audited for security or reliability issues due to its closed-source nature.\n\n"
        "Do you want to load firmware on this computer?"));
    descLabel->setWordWrap(true);

    noButton = new QRadioButton(tr("&No, do not load firmware on this computer."));
    yesButton = new QRadioButton(tr("&Yes, load firmware on this computer."));
    firmwareChoice = new QButtonGroup;
    firmwareChoice->addButton(noButton);
    firmwareChoice->addButton(yesButton);

    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(descLabel);
    layout->addStretch();
    layout->addWidget(noButton);
    layout->addWidget(yesButton);
    layout->addStretch();

    connect(firmwareChoice, static_cast<void (QButtonGroup:: *)(QAbstractButton *)>(&QButtonGroup::buttonClicked),
            [=](QAbstractButton *button) {
        if(button == yesButton) horizonWizard()->firmware = true;
        else horizonWizard()->firmware = false;
    });

    setLayout(layout);
}
