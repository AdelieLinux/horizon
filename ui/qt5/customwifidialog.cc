/*
 * customwifidialog.cc - Implementation of a dialog for entering a custom AP
 * horizon-qt5, the Qt 5 user interface for
 * Project Horizon
 *
 * Copyright (c) 2020 Adélie Linux and contributors.  All rights reserved.
 * This code is licensed under the AGPL 3.0 license, as noted in the
 * LICENSE-code file in the root directory of this repository.
 *
 * SPDX-License-Identifier: AGPL-3.0-only
 */

#include "customwifidialog.hh"

#include <QFormLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QVBoxLayout>

CustomWiFiDialog::CustomWiFiDialog(QWidget *parent)
    : QDialog(parent, Qt::WindowTitleHint | Qt::WindowCloseButtonHint) {
    setWindowTitle(tr("Add Wireless Network"));

    ok = new QPushButton(tr("Confirm"));
    ok->setEnabled(false);
    connect(ok, &QPushButton::clicked, this, &QDialog::accept);
    QPushButton *cancel = new QPushButton(tr("Cancel"));
    connect(cancel, &QPushButton::clicked, this, &QDialog::reject);

    QHBoxLayout *buttonLayout = new QHBoxLayout;
    buttonLayout->addWidget(ok, 0, Qt::AlignCenter);
    buttonLayout->addWidget(cancel, 0, Qt::AlignCenter);

    networkInput = new QLineEdit;
    networkInput->setWhatsThis(tr("The SSID of the network."));
    connect(networkInput, &QLineEdit::textChanged,
            this, &CustomWiFiDialog::ensureButton);

    /*: No security; open network */
    securityNone = new QRadioButton(tr("None"));
    securityWEP = new QRadioButton(tr("WEP"));
    securityWPA = new QRadioButton(tr("WPA/WPA2 Personal"));
    securityGroup = new QButtonGroup;
    securityGroup->setExclusive(true);
    securityGroup->addButton(securityNone, 0);
    securityGroup->addButton(securityWEP, 1);
    securityGroup->addButton(securityWPA, 2);
    connect(securityGroup,
            QOverload<QAbstractButton *>::of(&QButtonGroup::buttonClicked),
            this, &CustomWiFiDialog::ensureButton);

    QHBoxLayout *radioLayout = new QHBoxLayout;
    radioLayout->addWidget(securityNone, 0, Qt::AlignCenter);
    radioLayout->addWidget(securityWEP,  0, Qt::AlignCenter);
    radioLayout->addWidget(securityWPA,  0, Qt::AlignCenter);

    QFormLayout *controlLayout = new QFormLayout;
    controlLayout->addRow(new QLabel(tr("Network Name:")), networkInput);
    controlLayout->addRow(new QLabel(tr("Security Type:")), radioLayout);

    QVBoxLayout *mainBox = new QVBoxLayout;
    mainBox->addLayout(controlLayout);
    mainBox->addLayout(buttonLayout);

    setLayout(mainBox);
}

QString CustomWiFiDialog::networkName() const {
    return networkInput->text();
}

void CustomWiFiDialog::setNetworkName(const QString &network) {
    networkInput->setText(network);
}

QStringList CustomWiFiDialog::flags() const {
    switch(securityGroup->checkedId()) {
    default:
        return {};
    case 1:
        return {"WEP"};
    case 2:
        return {"WPA-PSK"};
    }
}

void CustomWiFiDialog::ensureButton() {
    ok->setEnabled(!networkInput->text().isEmpty() && securityGroup->checkedId() != -1);
}
