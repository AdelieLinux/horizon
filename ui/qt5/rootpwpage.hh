/*
 * rootpwpage.hh - Definition of the UI.Accounts.RootPW page
 * horizon-qt5, the Qt 5 user interface for
 * Project Horizon
 *
 * Copyright (c) 2019 Adélie Linux and contributors.  All rights reserved.
 * This code is licensed under the AGPL 3.0 license, as noted in the
 * LICENSE-code file in the root directory of this repository.
 *
 * SPDX-License-Identifier: AGPL-3.0-only
 */

#ifndef ROOTPWPAGE_HH
#define ROOTPWPAGE_HH

#include "horizonwizardpage.hh"

#include <QLineEdit>

class RootPassphrasePage : public HorizonWizardPage {
public:
    RootPassphrasePage(QWidget *parent = nullptr);
    bool isComplete() const;
private:
    QLineEdit *rootPW, *confirmPW;
};

#endif  /* !ROOTPWPAGE_HH */
