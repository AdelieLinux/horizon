/*
 * networkifacepage.cc - Implementation of the UI.Network.ChooseIface page
 * horizon-qt5, the Qt 5 user interface for
 * Project Horizon
 *
 * Copyright (c) 2019 Adélie Linux and contributors.  All rights reserved.
 * This code is licensed under the AGPL 3.0 license, as noted in the
 * LICENSE-code file in the root directory of this repository.
 *
 * SPDX-License-Identifier: AGPL-3.0-only
 */

#include "networkifacepage.hh"
#include "horizonwizard.hh"

#include <QLabel>
#include <QListView>
#include <QListWidget>
#include <QVBoxLayout>

NetworkIfacePage::NetworkIfacePage(QWidget *parent) :
    HorizonWizardPage(parent) {
    loadWatermark("network");
    setTitle(tr("Multiple Network Interfaces Detected"));
}

void NetworkIfacePage::initializePage() {
    QLabel *descLabel;
    QListWidget *ifaceList;
    QVBoxLayout *layout;

    descLabel = new QLabel(tr(
        "Your computer has more than one network interface device.  Select the one you wish to use during installation."));
    descLabel->setWordWrap(true);

    ifaceList = new QListWidget(this);
    for(auto &iface : horizonWizard()->interfaces) {
        QIcon ifaceIcon;
        QString ifaceDevName = QString::fromStdString(iface.first);
        QString ifaceName;

        switch(iface.second) {
        case HorizonWizard::Wireless:
            ifaceIcon = QIcon::fromTheme("network-wireless");
            ifaceName = tr("Wi-Fi (%1)").arg(ifaceDevName);
            break;
        case HorizonWizard::Ethernet:
            ifaceIcon = QIcon::fromTheme("network-wired");
            ifaceName = tr("Ethernet (%1)").arg(ifaceDevName);
            break;
        case HorizonWizard::Bonded:
            ifaceIcon = QIcon::fromTheme("network-card");
            ifaceName = tr("Bonded Interface (%1)").arg(ifaceDevName);
            break;
        case HorizonWizard::Unknown:
            ifaceIcon = QIcon::fromTheme("network-card");
            ifaceName = tr("Unknown Interface (%1)").arg(ifaceDevName);
            break;
        }

        new QListWidgetItem(ifaceIcon, ifaceName, ifaceList);
    }

    ifaceList->setViewMode(QListView::IconMode);

    layout = new QVBoxLayout;
    layout->addWidget(descLabel);
    layout->addWidget(ifaceList);
    setLayout(layout);
}
