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

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>

NetManualPage::NetManualPage(QWidget *parent) : HorizonWizardPage(parent) {
    loadWatermark("network");
    setTitle(tr("Configure Your Network"));

    QLabel *descLabel = new QLabel(tr("Input the details for your network connection."));

    QVBoxLayout *v4Pane = new QVBoxLayout;
    useV4 = new QCheckBox(tr("Enable IPv&4"));
    useV4->setChecked(true);
    v4Addr = new QLineEdit;
    v4Addr->setInputMask("900.900.900.900;_");
    v4Gateway = new QLineEdit;
    v4Gateway->setInputMask("900.900.900.900;_");
    v4DNS = new QLineEdit;
    v4DNS->setInputMask("900.900.900.900;_");
    v4DNS->setText("9.9.9.9");

    connect(useV4, &QCheckBox::toggled, [=](bool ticked) {
        v4Addr->setEnabled(ticked);
        v4Gateway->setEnabled(ticked);
        v4DNS->setEnabled(ticked);
        this->horizonWizard()->ipv4.use = ticked;
        emit completeChanged();
    });
    connect(v4Addr, &QLineEdit::textEdited, [=] {
        this->horizonWizard()->ipv4.address = v4Addr->text();
        emit completeChanged();
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
    v4Pane->addWidget(new QLabel(tr("IPv4 Gateway:")));
    v4Pane->addWidget(v4Gateway);
    v4Pane->addWidget(new QLabel(tr("IPv4 DNS Server:")));
    v4Pane->addWidget(v4DNS);

    QVBoxLayout *v6Pane = new QVBoxLayout;
    useV6 = new QCheckBox(tr("Enable IPv&6"));
    useV6->setChecked(true);
    v6Addr = new QLineEdit;
    v6Gateway = new QLineEdit;
    v6DNS = new QLineEdit;
    v6DNS->setText("2620:fe::fe");

    connect(useV6, &QCheckBox::toggled, [=](bool ticked) {
        v6Addr->setEnabled(ticked);
        v6Gateway->setEnabled(ticked);
        v6DNS->setEnabled(ticked);
        this->horizonWizard()->ipv6.use = true;
        emit completeChanged();
    });
    connect(v6Addr, &QLineEdit::textEdited, [=] {
        this->horizonWizard()->ipv6.address = v6Addr->text();
        emit completeChanged();
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
    if(horizonWizard()->interfaces.size() != 1) {
        ifaceWidget->show();
    } else {
        ifaceWidget->hide();
    }

    this->horizonWizard()->ipv4.use = true;
    this->horizonWizard()->ipv4.dns = "9.9.9.9";
    this->horizonWizard()->ipv6.use = true;
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
