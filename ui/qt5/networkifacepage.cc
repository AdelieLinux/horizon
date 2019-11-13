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

#ifdef HAS_INSTALL_ENV
#   include <net/if.h>      /* ifreq */
#   include <sys/ioctl.h>   /* ioctl */
#   include <unistd.h>      /* close */
#endif

#include <algorithm>
#include <QDebug>
#include <QLabel>
#include <QListView>
#include <QVBoxLayout>

NetworkIfacePage::NetworkIfacePage(QWidget *parent) :
    HorizonWizardPage(parent) {
    loadWatermark("network");
    setTitle(tr("Multiple Network Interfaces Detected"));

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
    ifaceList->setWhatsThis(tr("A list of network interfaces present in your computer.  Select the interface you wish to use for installation."));
}

void NetworkIfacePage::initializePage() {
    QLabel *descLabel;
    QVBoxLayout *layout;

    descLabel = new QLabel(tr(
        "Your computer has more than one network interface device.  Select the one you wish to use during installation."));
    descLabel->setWordWrap(true);

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
            ifaceName = tr("Bond (%1)").arg(ifaceDevName);
            break;
        case HorizonWizard::Unknown:
            ifaceIcon = QIcon::fromTheme("network-card");
            ifaceName = ifaceDevName;
            break;
        }

        QListWidgetItem *item = new QListWidgetItem(ifaceIcon, ifaceName,
                                                    ifaceList);
        /* Retrieving the index is always valid, and is not even privileged. */
        struct ifreq request;
        int my_sock = ::socket(AF_INET, SOCK_STREAM, 0);
        if(my_sock == -1) {
            continue;
        }
        memset(&request, 0, sizeof(request));
        memcpy(&request.ifr_name, iface.first.c_str(), iface.first.size());
        errno = 0;
        if(ioctl(my_sock, SIOCGIFHWADDR, &request) != -1) {
            char *buf;
            asprintf(&buf, "%02X:%02X:%02X:%02X:%02X:%02X",
                     request.ifr_ifru.ifru_hwaddr.sa_data[0],
                     request.ifr_ifru.ifru_hwaddr.sa_data[1],
                     request.ifr_ifru.ifru_hwaddr.sa_data[2],
                     request.ifr_ifru.ifru_hwaddr.sa_data[3],
                     request.ifr_ifru.ifru_hwaddr.sa_data[4],
                     request.ifr_ifru.ifru_hwaddr.sa_data[5]);
            item->setToolTip(QString(buf));
            free(buf);
        }
        ::close(my_sock);
    }

    layout = new QVBoxLayout;
    layout->addWidget(descLabel);
    layout->addWidget(ifaceList);
    setLayout(layout);
}

bool NetworkIfacePage::isComplete() const {
    return ifaceList->currentRow() != -1;
}

int NetworkIfacePage::nextId() const {
    if(ifaceList->currentRow() < 0) return HorizonWizard::Page_Network_Iface;

    auto iterator = horizonWizard()->interfaces.begin();
    std::advance(iterator, ifaceList->currentRow());

    switch(iterator->second) {
    case HorizonWizard::Wireless:
        return HorizonWizard::Page_Network_Wireless;
    default:
        return HorizonWizard::Page_Network_DHCP;
    }
}
