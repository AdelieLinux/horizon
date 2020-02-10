/*
 * subnetbox.cc - Implementation of a widget for inputting an IPv4 subnet mask
 * horizon-qt5, the Qt 5 user interface for
 * Project Horizon
 *
 * Copyright (c) 2020 Adélie Linux and contributors.  All rights reserved.
 * This code is licensed under the AGPL 3.0 license, as noted in the
 * LICENSE-code file in the root directory of this repository.
 *
 * SPDX-License-Identifier: AGPL-3.0-only
 */

#include "subnetbox.hh"
#include "util/net.hh"

SubnetBox::SubnetBox(QWidget *parent) : QWidget(parent) {
    SubnetBox::layout = new QHBoxLayout(this);
    SubnetBox::subnet = new QLineEdit(this);
    SubnetBox::cidr   = new QSpinBox(this);

    this->setLayout(layout);

    layout->addWidget(subnet);
    layout->addWidget(cidr);
    layout->setMargin(0);

    subnet->setInputMask("000.000.000.000;_");
    cidr->setRange(1, 32);

    connect(subnet, &QLineEdit::textEdited, this, &SubnetBox::subnetEdited);
    connect(cidr, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &SubnetBox::cidrEdited);
}

QString SubnetBox::subnetMask(void) const {
    return subnet->text();
}

int SubnetBox::subnetCIDR(void) const {
    return cidr->value();
}

void SubnetBox::setSubnetCIDR(int value) {
    cidr->setValue(value);
    cidrEdited(value);
}

void SubnetBox::subnetEdited(const QString &text) {
    int value = subnet_mask_to_cidr(text.toUtf8());
    if(value > 0) {
        cidr->setValue(value);
        emit valueChanged(value);
    }
}

void SubnetBox::cidrEdited(int value) {
    int bytes = value / 8, bits = value % 8, lastfilled = 0;
    QString data;
    char temp[4];

    for(int i = 0; i < bytes; i++) {
        data.append("255.");
    }
    for(int i = bits; i > 0; i--) {
        lastfilled |= (1 << (8 - i));
    }
    snprintf(temp, 4, "%d", lastfilled);
    data.append(temp);
    for(int i = 3; i > bytes; i--) {
        data.append(".0");
    }

    subnet->setText(data);

    emit valueChanged(value);
}
