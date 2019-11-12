/*
 * netsimplewifipage.cc - Implementation of the UI.Network.Wireless page
 * horizon-qt5, the Qt 5 user interface for
 * Project Horizon
 *
 * Copyright (c) 2019 Adélie Linux and contributors.  All rights reserved.
 * This code is licensed under the AGPL 3.0 license, as noted in the
 * LICENSE-code file in the root directory of this repository.
 *
 * SPDX-License-Identifier: AGPL-3.0-only
 */

#include "netsimplewifipage.hh"

#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>

NetworkSimpleWirelessPage::NetworkSimpleWirelessPage(QWidget *parent)
    : HorizonWizardPage(parent) {
    QVBoxLayout *layout;
    QLabel *statusLabel;
    QPushButton *rescanButton;

    loadWatermark("network");
    setTitle(tr("Select Your Network"));

    statusLabel = new QLabel(tr("Scanning for networks..."));

    rescanButton = new QPushButton(tr("&Rescan Networks"));
    rescanButton->setEnabled(false);
    connect(rescanButton, &QPushButton::clicked, [=](void) { doScan(); });

    ssidListView = new QListWidget;

    passphrase = new QLineEdit(this);
    passphrase->setEchoMode(QLineEdit::Password);
    passphrase->setPlaceholderText(tr("Passphrase"));
    passphrase->hide();

    layout = new QVBoxLayout;
    layout->addWidget(statusLabel, 0, Qt::AlignCenter);
    layout->addSpacing(10);
    layout->addWidget(ssidListView, 0, Qt::AlignCenter);
    layout->addWidget(rescanButton);
    layout->addSpacing(10);
    layout->addWidget(passphrase);
    setLayout(layout);
}

void NetworkSimpleWirelessPage::doScan() {
    ssidListView->clear();
}

void NetworkSimpleWirelessPage::initializePage() {
    doScan();
}

int NetworkSimpleWirelessPage::nextId() const {
    return HorizonWizard::Page_Network_DHCP;
}
