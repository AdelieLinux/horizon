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
        "<p>Your computer may require the use of drivers which use proprietary, closed-source components (or <i>firmware</i>) in order to use certain hardware or functionality.</p>"
        "<p>Most Wi-Fi network adaptors and 3D graphics cards require proprietary firmware.</p>"
        "<p>Proprietary firmware cannot be audited for security or reliability issues due to its closed-source nature.  Only install proprietary firmware if you require it.</p>"
        "<p>If you intend to use this computer to perform security-sensitive tasks, we strongly recommend that you choose not to load firmware on this computer.</p>"
        "<p>Do you want to load firmware on this computer?</p>"));
    descLabel->setTextFormat(Qt::RichText);
    descLabel->setWordWrap(true);

    noButton = new QRadioButton(tr("&No, do not load firmware on this computer."));
    noButton->setChecked(true);
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

    setField("firmware", QVariant(false));
    connect(firmwareChoice, static_cast<void (QButtonGroup:: *)(QAbstractButton *)>(&QButtonGroup::buttonClicked),
            [=](QAbstractButton *button) {
        if(button == yesButton) setField("firmware", QVariant(true));
        else setField("firmware", QVariant(false));
    });

    setLayout(layout);
}
