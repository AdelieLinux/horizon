/*
 * netmanualpage.hh - Definition of the UI.Network.Manual page
 * horizon-qt5, the Qt 5 user interface for
 * Project Horizon
 *
 * Copyright (c) 2020 Adélie Linux and contributors.  All rights reserved.
 * This code is licensed under the AGPL 3.0 license, as noted in the
 * LICENSE-code file in the root directory of this repository.
 *
 * SPDX-License-Identifier: AGPL-3.0-only
 */

#ifndef NETMANUALPAGE_HH
#define NETMANUALPAGE_HH

#include "horizonwizardpage.hh"

#include <QCheckBox>
#include <QLineEdit>
#include <QSpinBox>
#include "subnetbox.hh"

class NetManualPage : public HorizonWizardPage {
public:
    NetManualPage(QWidget *parent = nullptr);

    void initializePage() override;
    bool isComplete() const override;
private:
    QWidget *ifaceWidget;
    QCheckBox *useV6, *useV4;

    QLineEdit *v6Addr, *v6Gateway, *v6DNS;
    QSpinBox *v6Prefix;
    QLineEdit *v4Addr, *v4Gateway, *v4DNS;
    SubnetBox *v4Prefix;
};

#endif /* !NETMANUALPAGE_HH */
