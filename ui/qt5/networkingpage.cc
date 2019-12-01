/*
 * networkingpage.cc - Implementation of the UI.Network.AddressType page
 * horizon-qt5, the Qt 5 user interface for
 * Project Horizon
 *
 * Copyright (c) 2019 Adélie Linux and contributors.  All rights reserved.
 * This code is licensed under the AGPL 3.0 license, as noted in the
 * LICENSE-code file in the root directory of this repository.
 *
 * SPDX-License-Identifier: AGPL-3.0-only
 */

#include "networkingpage.hh"
#include "horizonwizard.hh"

#include <QLabel>
#include <QVBoxLayout>

NetworkingPage::NetworkingPage(QWidget *parent) : HorizonWizardPage(parent) {
    loadWatermark("network");
    setTitle(tr("Networking Setup"));
}

void NetworkingPage::initializePage() {
    QLabel *descLabel;
    QVBoxLayout *layout;

    if(horizonWizard()->interfaces.empty()) {
        descLabel = new QLabel(tr(
            "No supported network interfaces have been detected on your computer.\n\n"
            "You will not be able to connect to a network nor the Internet.\n\n"
            "If you have a network interface attached to your computer, it may not be supported by Adélie Linux.  Please contact our community at https://help.adelielinux.org/ for help."));
    } else {
        descLabel = new QLabel(tr(
            "If your computer is directly connected to the Internet via Ethernet or Wi-Fi using a modem or router, choose Automatic.\n\n"
            "If you need to set a static IP address, or you use a VPN or proxy server, choose Manual.\n\n"
            "If you don't want to configure networking or you don't want to use this computer on the Internet, choose Skip."));
    }
    descLabel->setWordWrap(true);

    if(!horizonWizard()->interfaces.empty()) {
        simple = new QRadioButton(tr("&Automatic - my computer connects to the Internet directly\n"
                                     "or via a modem/router."));
        advanced = new QRadioButton(tr("&Manual - my computer connects to an enterprise network,\n"
                                       "or I use a static IP address, VPN, or 802.1X."));
    }
    skip = new QRadioButton(tr("&Skip - I don't want to connect to a network or the Internet."));

    radioGroup = new QButtonGroup(this);
    if(!horizonWizard()->interfaces.empty()) {
        radioGroup->addButton(simple);
        radioGroup->addButton(advanced);
    }
    radioGroup->addButton(skip);

    QObject::connect(radioGroup, static_cast<void (QButtonGroup:: *)(QAbstractButton *)>(&QButtonGroup::buttonClicked),
                     [=](QAbstractButton *button) {
        if(button == skip) {
            horizonWizard()->network = false;
        } else {
            horizonWizard()->network = true;
            if(button == simple) {
                horizonWizard()->net_dhcp = true;
            } else {
                horizonWizard()->net_dhcp = false;
            }
        }

        emit completeChanged();
    });

    layout = new QVBoxLayout;
    layout->addWidget(descLabel);
    layout->addSpacing(50);
    if(!horizonWizard()->interfaces.empty()) {
        layout->addWidget(simple);
        layout->addWidget(advanced);
    }
    layout->addWidget(skip);
    setLayout(layout);
}

bool NetworkingPage::isComplete() const {
    return (radioGroup->checkedButton() != nullptr);
}

int NetworkingPage::nextId() const {
    if(radioGroup->checkedButton() == simple) {
        if(horizonWizard()->interfaces.size() != 1) {
            return HorizonWizard::Page_Network_Iface;
        } else {
            horizonWizard()->chosen_auto_iface =
                    (horizonWizard()->interfaces.begin())->first;
            if((horizonWizard()->interfaces.begin())->second.type
                    == HorizonWizard::Wireless) {
                return HorizonWizard::Page_Network_Wireless;
            } else {
                return HorizonWizard::Page_Network_DHCP;
            }
        }
    } else if(radioGroup->checkedButton() == advanced) {
        return HorizonWizard::Page_Network_Manual;
    } else {
        /* REQ: UI.Network.AddressType.Skip */
        return HorizonWizard::Page_DateTime;
    }
}
