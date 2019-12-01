/*
 * networkifacepage.hh - Definition of the UI.Network.ChooseIface page
 * horizon-qt5, the Qt 5 user interface for
 * Project Horizon
 *
 * Copyright (c) 2019 Adélie Linux and contributors.  All rights reserved.
 * This code is licensed under the AGPL 3.0 license, as noted in the
 * LICENSE-code file in the root directory of this repository.
 *
 * SPDX-License-Identifier: AGPL-3.0-only
 */

#ifndef NETWORKIFACEPAGE_HH
#define NETWORKIFACEPAGE_HH

#ifdef HAS_INSTALL_ENV
#include <QListWidget>
#else
#include <QLineEdit>
#endif  /* HAS_INSTALL_ENV */

#include "horizonwizardpage.hh"

class NetworkIfacePage : public HorizonWizardPage {
public:
    NetworkIfacePage(QWidget *parent = nullptr);
    void initializePage() override;
    bool isComplete() const override;
    int nextId() const override;
    bool validatePage() override;
private:
#ifdef HAS_INSTALL_ENV
    QListWidget *ifaceList;
#else
    QLineEdit *ifaceName;
#endif  /* HAS_INSTALL_ENV */
};

#endif /* !NETWORKIFACEPAGE_HH */
