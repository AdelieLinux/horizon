/*
 * netmanualpage.cc - Implementation of the UI.Network.Manual page
 * horizon-qt5, the Qt 5 user interface for
 * Project Horizon
 *
 * Copyright (c) 2020 Adélie Linux and contributors.  All rights reserved.
 * This code is licensed under the AGPL 3.0 license, as noted in the
 * LICENSE-code file in the root directory of this repository.
 *
 * SPDX-License-Identifier: AGPL-3.0-only
 */

#include "netmanualpage.hh"

#include <algorithm>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QComboBox>
#include <QLabel>

NetManualPage::NetManualPage(QWidget *parent) : HorizonWizardPage(parent) {
    loadWatermark("network");
    setTitle(tr("Configure Your Network"));

    QLabel *descLabel = new QLabel(tr("Input the details for your network connection."));

    QVBoxLayout *v4Pane = new QVBoxLayout;
    useV4 = new QCheckBox(tr("Enable IPv&4"));
    useV4->setChecked(true);
    useV4->setWhatsThis(tr("Enables the IPv4 protocol for network connectivity."));
    v4Addr = new QLineEdit;
    v4Addr->setInputMask("900.900.900.900;_");
    v4Addr->setWhatsThis(tr("The IPv4 address for this computer."));
    v4Prefix = new SubnetBox;
    v4Prefix->setSubnetCIDR(24);
    v4Prefix->setWhatsThis(tr("The subnet prefix/mask for this computer's IPv4 network."));
    v4Gateway = new QLineEdit;
    v4Gateway->setInputMask("900.900.900.900;_");
    v4Gateway->setWhatsThis(tr("The default gateway for IPv4 traffic."));
    v4DNS = new QLineEdit;
    v4DNS->setInputMask("900.900.900.900;_");
    v4DNS->setText("9.9.9.9");
    v4DNS->setWhatsThis(tr("The DNS server to use for IPv4 name resolution."));

    connect(useV4, &QCheckBox::toggled, [=](bool ticked) {
        v4Addr->setEnabled(ticked);
        v4Prefix->setEnabled(ticked);
        v4Gateway->setEnabled(ticked);
        v4DNS->setEnabled(ticked);
        this->horizonWizard()->ipv4.use = ticked;
        emit completeChanged();
    });
    connect(v4Addr, &QLineEdit::textEdited, [=] {
        this->horizonWizard()->ipv4.address = v4Addr->text();
        emit completeChanged();
    });
    connect(v4Prefix, &SubnetBox::valueChanged, [=] {
        this->horizonWizard()->ipv4.mask = QString::number(v4Prefix->subnetCIDR());
    });
    connect(v4Gateway, &QLineEdit::textEdited, [=] {
        this->horizonWizard()->ipv4.gateway = v4Gateway->text();
        emit completeChanged();
    });
    connect(v4DNS, &QLineEdit::textEdited, [=] {
        this->horizonWizard()->ipv4.dns = v4DNS->text();
        emit completeChanged();
    });

    v4Pane->addWidget(useV4);
    v4Pane->addSpacing(20);
    v4Pane->addWidget(new QLabel(tr("IPv4 Address:")));
    v4Pane->addWidget(v4Addr);
    v4Pane->addWidget(new QLabel(tr("IPv4 Subnet:")));
    v4Pane->addWidget(v4Prefix);
    v4Pane->addWidget(new QLabel(tr("IPv4 Gateway:")));
    v4Pane->addWidget(v4Gateway);
    v4Pane->addWidget(new QLabel(tr("IPv4 DNS Server:")));
    v4Pane->addWidget(v4DNS);

    QVBoxLayout *v6Pane = new QVBoxLayout;
    useV6 = new QCheckBox(tr("Enable IPv&6"));
    useV6->setChecked(true);
    useV6->setWhatsThis(tr("Enables the IPv6 protocol for network connectivity."));
    v6Addr = new QLineEdit;
    v6Addr->setWhatsThis(tr("The IPv6 address for this computer."));
    v6Prefix = new QSpinBox;
    v6Prefix->setRange(1, 128);
    v6Prefix->setValue(64);
    v6Prefix->setWhatsThis(tr("The subnet prefix for this computer's IPv6 network."));
    v6Gateway = new QLineEdit;
    v6Gateway->setWhatsThis(tr("The default gateway for IPv6 traffic."));
    v6DNS = new QLineEdit;
    v6DNS->setText("2620:fe::fe");
    v4DNS->setWhatsThis(tr("The DNS server to use for IPv6 name resolution."));

    connect(useV6, &QCheckBox::toggled, [=](bool ticked) {
        v6Addr->setEnabled(ticked);
        v6Prefix->setEnabled(ticked);
        v6Gateway->setEnabled(ticked);
        v6DNS->setEnabled(ticked);
        this->horizonWizard()->ipv6.use = ticked;
        emit completeChanged();
    });
    connect(v6Addr, &QLineEdit::textEdited, [=] {
        this->horizonWizard()->ipv6.address = v6Addr->text();
        emit completeChanged();
    });
    connect(v6Prefix, QOverload<int>::of(&QSpinBox::valueChanged), [=] {
        this->horizonWizard()->ipv6.mask = QString::number(v6Prefix->value());
    });
    connect(v6Gateway, &QLineEdit::textEdited, [=] {
        this->horizonWizard()->ipv6.gateway = v6Gateway->text();
        emit completeChanged();
    });
    connect(v6DNS, &QLineEdit::textEdited, [=] {
        this->horizonWizard()->ipv6.dns = v6DNS->text();
        emit completeChanged();
    });

    v6Pane->addWidget(useV6);
    v6Pane->addSpacing(20);
    v6Pane->addWidget(new QLabel(tr("IPv6 Address:")));
    v6Pane->addWidget(v6Addr);
    v6Pane->addWidget(new QLabel(tr("IPv6 Subnet Prefix:")));
    v6Pane->addWidget(v6Prefix);
    v6Pane->addWidget(new QLabel(tr("IPv6 Gateway:")));
    v6Pane->addWidget(v6Gateway);
    v6Pane->addWidget(new QLabel(tr("IPv6 DNS Server:")));
    v6Pane->addWidget(v6DNS);

    QHBoxLayout *paneLayout = new QHBoxLayout;
    paneLayout->addLayout(v4Pane);
    paneLayout->addLayout(v6Pane);

    QVBoxLayout *overallLayout = new QVBoxLayout;
    overallLayout->addWidget(descLabel);

    ifaceWidget = new QWidget;
    QHBoxLayout *ifaceLayout = new QHBoxLayout;
    ifaceWidget->setLayout(ifaceLayout);

    overallLayout->addWidget(ifaceWidget);
    overallLayout->addStretch();
    overallLayout->addLayout(paneLayout);
    overallLayout->addStretch();
    setLayout(overallLayout);
}

void NetManualPage::initializePage() {
    if(horizonWizard()->interfaces.size() > 1) {
        /* The first interface will either be the only one,
         * or the default in the list. */
        auto iface = horizonWizard()->interfaces.begin();
        horizonWizard()->chosen_auto_iface = iface->first;
    }

#ifdef HAS_INSTALL_ENV
    if(horizonWizard()->interfaces.size() != 1)
    {
        QComboBox *ifaceBox = new QComboBox;
        ifaceBox->setWhatsThis(tr("The network interface to use for this computer's primary network connection.  You may configure additional interfaces after installation."));
        ifaceWidget->layout()->addWidget(new QLabel(tr("Use interface: ")));
        ifaceWidget->layout()->addWidget(ifaceBox);

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
            ifaceBox->addItem(ifaceIcon, ifaceName);
        }
        connect(ifaceBox, QOverload<int>::of(&QComboBox::currentIndexChanged), [=](int index) {
            auto iterator = horizonWizard()->interfaces.begin();
            std::advance(iterator, index);
            horizonWizard()->chosen_auto_iface = iterator->first;
        });
    } else {
        auto iface = horizonWizard()->interfaces.begin();
        ifaceWidget->layout()->addWidget(new QLabel(
            tr("Configuring interface %1 (%2)")
            .arg(QString::fromStdString(iface->first))
            .arg(iface->second.mac)));
    }
#else   /* !HAS_INSTALL_ENV */
    QLineEdit *ifaceBox = new QLineEdit;
    ifaceBox->setPlaceholderText(tr("Interface name (eth0, wlan0, ...)"));
    ifaceWidget->layout()->addWidget(new QLabel(tr("Use interface: ")));
    ifaceWidget->layout()->addWidget(ifaceBox);
    connect(ifaceBox, &QLineEdit::textEdited, [=] {
        horizonWizard()->chosen_auto_iface = ifaceBox->text().toStdString();
    });
#endif  /* HAS_INSTALL_ENV */

    this->horizonWizard()->ipv4.use = true;
    this->horizonWizard()->ipv4.mask = QString::number(24);
    this->horizonWizard()->ipv4.dns = "9.9.9.9";
    this->horizonWizard()->ipv6.use = true;
    this->horizonWizard()->ipv6.mask = QString::number(64);
    this->horizonWizard()->ipv6.dns = "2620:fe::fe";
}

#include <iostream>

bool NetManualPage::isComplete() const {
    /* REQ: UI.Network.Manual.Enable: At least one must be ticked. */
    if(!useV6->isChecked() && !useV4->isChecked()) return false;

    bool valid = true;

    if(useV6->isChecked()) {
        valid = !v6Addr->text().isEmpty() && v6Addr->hasAcceptableInput() &&
                !v6DNS->text().isEmpty() && v6DNS->hasAcceptableInput();
    }

    /* If both are checked, we just mix in v6 validity. */
    if(useV4->isChecked()) {
        return !v4Addr->text().isEmpty() && v4Addr->hasAcceptableInput() &&
               !v4DNS->text().isEmpty() && v4DNS->hasAcceptableInput() &&
                valid;
    }

    /* Okay, only v6 matters. */
    return valid;
}
