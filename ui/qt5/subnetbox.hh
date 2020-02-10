/*
 * subnetbox.hh - Definition of a widget for inputting an IPv4 subnet mask
 * horizon-qt5, the Qt 5 user interface for
 * Project Horizon
 *
 * Copyright (c) 2020 Adélie Linux and contributors.  All rights reserved.
 * This code is licensed under the AGPL 3.0 license, as noted in the
 * LICENSE-code file in the root directory of this repository.
 *
 * SPDX-License-Identifier: AGPL-3.0-only
 */

#ifndef SUBNETBOX_HH
#define SUBNETBOX_HH

#include <QWidget>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QSpinBox>
#include <QString>

class SubnetBox : public QWidget {
    Q_OBJECT

public:
    SubnetBox(QWidget *parent = nullptr);
    QString subnetMask() const;
    int subnetCIDR() const;
    void setSubnetCIDR(int value);

public slots:
    void subnetEdited(const QString &text);
    void cidrEdited(int value);

signals:
    void valueChanged(int value);

private:
    QHBoxLayout *layout;
    QLineEdit *subnet;
    QSpinBox *cidr;
};

#endif
