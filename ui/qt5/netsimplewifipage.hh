/*
 * netsimplewifipage.hh - Definition of the UI.Network.Wireless page
 * horizon-qt5, the Qt 5 user interface for
 * Project Horizon
 *
 * Copyright (c) 2019 Adélie Linux and contributors.  All rights reserved.
 * This code is licensed under the AGPL 3.0 license, as noted in the
 * LICENSE-code file in the root directory of this repository.
 *
 * SPDX-License-Identifier: AGPL-3.0-only
 */

#ifndef NETWORKSIMPLEWIRELESSPAGE_HH
#define NETWORKSIMPLEWIRELESSPAGE_HH

#include "horizonwizardpage.hh"

#include <QComboBox>
#include <QLineEdit>
#include <QListWidget>

class NetworkSimpleWirelessPage : public HorizonWizardPage {
public:
    NetworkSimpleWirelessPage(QWidget *parent = nullptr);

    void initializePage();
    void doScan();
    int nextId() const;
private:
    QListWidget *ssidListView;
    QLineEdit *passphrase;
};

#endif /* !NETWORKSIMPLEWIRELESSPAGE_HH */
