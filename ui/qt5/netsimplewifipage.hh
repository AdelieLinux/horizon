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

#ifdef HAS_INSTALL_ENV
#   include <bcnm/wpactrl.h>
#   include <QSocketNotifier>
#endif  /* HAS_INSTALL_ENV */

#include <QComboBox>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QProgressDialog>
#include <QPushButton>

class NetworkSimpleWirelessPage : public HorizonWizardPage {
public:
    NetworkSimpleWirelessPage(QWidget *parent = nullptr);
    ~NetworkSimpleWirelessPage();

    void initializePage();
    bool isComplete() const;
    int nextId() const;
    bool validatePage();
private:
    QLabel *statusLabel, *passLabel;
    QPushButton *addNetButton;

    QListWidget *ssidListView;
    QLineEdit *passphrase;

    void networkChosen(QListWidgetItem*, QListWidgetItem*);

#ifdef HAS_INSTALL_ENV
    wpactrl_t control;
    wpactrl_xchg_t exchange;
    wpactrl_xchgitem_t exchange_item;
    QPushButton *rescanButton;
    QSocketNotifier *notify, *connNotify;
    QProgressDialog *dialog;
    bool associated;

    void associate();
    void doScan();
    void processAssociateMessage(int);
    void scanDone(QString message);
    int processScan(wpactrl_t *, const char *, size_t);
    friend int scanResults(wpactrl_t *, char const *, size_t, void *, tain_t *);
#endif  /* HAS_INSTALL_ENV */
};

#endif /* !NETWORKSIMPLEWIRELESSPAGE_HH */
