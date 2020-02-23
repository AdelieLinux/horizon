/*
 * accountpage.hh - Definition of the UI.Accounts.UserAcct page
 * horizon-qt5, the Qt 5 user interface for
 * Project Horizon
 *
 * Copyright (c) 2019 Adélie Linux and contributors.  All rights reserved.
 * This code is licensed under the AGPL 3.0 license, as noted in the
 * LICENSE-code file in the root directory of this repository.
 *
 * SPDX-License-Identifier: AGPL-3.0-only
 */

#ifndef ACCOUNTPAGE_HH
#define ACCOUNTPAGE_HH

#include "horizonwizardpage.hh"
#include "useraccountwidget.hh"

#include <array>

class AccountPage : public HorizonWizardPage {
public:
    AccountPage(QWidget *parent = nullptr);
    bool isComplete() const override;
#ifdef HAS_INSTALL_ENV
    bool validatePage() override;
#endif
    std::array<UserAccountWidget *, 4> accountWidgets;
};

#endif  /* !ACCOUNTPAGE_HH */
