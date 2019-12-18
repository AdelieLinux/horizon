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
#include "netdhcppage.hh"

#include <algorithm>
#include <QDebug>
#include <QLabel>
#include <QListView>
#include <QVBoxLayout>

NetworkIfacePage::NetworkIfacePage(QWidget *parent) :
    HorizonWizardPage(parent) {
    loadWatermark("network");
#ifdef HAS_INSTALL_ENV
    setTitle(tr("Multiple Network Devices Detected"));

    ifaceList = new QListWidget(this);
    connect(ifaceList, &QListWidget::currentRowChanged, [=](int row) {
        emit completeChanged();
        if(row == -1) return;
        auto iterator = horizonWizard()->interfaces.begin();
        std::advance(iterator, row);
        horizonWizard()->chosen_auto_iface = iterator->first;
    });

    ifaceList->setGridSize(QSize(160, 128));
    ifaceList->setIconSize(QSize(96, 96));
    ifaceList->setViewMode(QListView::IconMode);
    ifaceList->setWhatsThis(tr("This is a list of network devices available in your computer.  Select the network device you wish to use for installation."));
#else
    setTitle(tr("Enter Network Device Name"));

    ifaceName = new QLineEdit(this);
    ifaceName->setPlaceholderText(tr("Network Device Name (i.e. eth0)"));
    ifaceName->setWhatsThis(tr("Enter the name of the computer's network device.  For example, eth0."));
    connect(ifaceName, &QLineEdit::textEdited, [=]{
        emit completeChanged();
        horizonWizard()->chosen_auto_iface = ifaceName->text().toStdString();
    });
#endif
}

void NetworkIfacePage::initializePage() {
    QLabel *descLabel;
    QVBoxLayout *layout;

#ifdef HAS_INSTALL_ENV
    descLabel = new QLabel(tr(
        "Your computer has more than one network device.  Select the one you wish to use during installation."));

    for(auto &iface : horizonWizard()->interfaces) {
        QIcon ifaceIcon;
        QString ifaceDevName = QString::fromStdString(iface.first);
        QString ifaceName;

        switch(iface.second.type) {
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
            ifaceName = tr("Bond (%1)").arg(ifaceDevName);
            break;
        case HorizonWizard::Unknown:
            ifaceIcon = QIcon::fromTheme("network-card");
            ifaceName = ifaceDevName;
            break;
        }

        QListWidgetItem *item = new QListWidgetItem(ifaceIcon, ifaceName,
                                                    ifaceList);
        item->setToolTip(iface.second.mac);
    }
#else
    descLabel = new QLabel(tr("Enter the name of the network device you will use on this computer."));
#endif  /* HAS_INSTALL_ENV */
    descLabel->setWordWrap(true);

    layout = new QVBoxLayout;
    layout->addWidget(descLabel);
#ifdef HAS_INSTALL_ENV
    layout->addWidget(ifaceList);
#else
    layout->addStretch();
    layout->addWidget(ifaceName);
    layout->addStretch();
#endif  /* HAS_INSTALL_ENV */
    setLayout(layout);
}

bool NetworkIfacePage::isComplete() const {
#ifdef HAS_INSTALL_ENV
    return ifaceList->currentRow() != -1;
#else
    return ifaceName->text().size() > 0;
#endif  /* HAS_INSTALL_ENV */
}

int NetworkIfacePage::nextId() const {
#ifdef HAS_INSTALL_ENV
    if(ifaceList->currentRow() < 0) return HorizonWizard::Page_Network_Iface;

    auto iterator = horizonWizard()->interfaces.begin();
    std::advance(iterator, ifaceList->currentRow());

    switch(iterator->second.type) {
    case HorizonWizard::Wireless:
        return HorizonWizard::Page_Network_Wireless;
    default:
        return HorizonWizard::Page_Network_DHCP;
    }
#else
    return HorizonWizard::Page_DateTime;
#endif  /* HAS_INSTALL_ENV */
}

bool NetworkIfacePage::validatePage() {
#ifdef HAS_INSTALL_ENV
    /* What a hack!
     *
     * Independent Pages means the DHCP page is never cleaned, even when Back
     * is chosen.  So, we have to do it from here. */
    horizonWizard()->removePage(HorizonWizard::Page_Network_DHCP);
    horizonWizard()->setPage(HorizonWizard::Page_Network_DHCP, new NetDHCPPage);
#endif  /* HAS_INSTALL_ENV */
    return true;
}
