/*
 * accountpage.cc - Implementation of the UI.Accounts.UserAcct page
 * horizon-qt5, the Qt 5 user interface for
 * Project Horizon
 *
 * Copyright (c) 2019 Adélie Linux and contributors.  All rights reserved.
 * This code is licensed under the AGPL 3.0 license, as noted in the
 * LICENSE-code file in the root directory of this repository.
 *
 * SPDX-License-Identifier: AGPL-3.0-only
 */

#include "accountpage.hh"
#ifdef HAS_INSTALL_ENV
#   include "commitpage.hh"
#endif  /* HAS_INSTALL_ENV */

#include <algorithm>
#include <QLabel>
#include <QVBoxLayout>

AccountPage::AccountPage(QWidget *parent) : HorizonWizardPage(parent) {
    setTitle(tr("User Accounts"));
    loadWatermark("acct");
    QLabel *descLabel = new QLabel(tr(
        "Enter the details of up to four people who will use this computer.  "
        "You can add more later with the User Manager.  "
        "For more information, consult the User's Handbook."));
    descLabel->setWordWrap(true);

    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(descLabel);

    accountWidgets[0] = new UserAccountWidget;
    accountWidgets[0]->setAdmin(true);
    accountWidgets[1] = new UserAccountWidget;
    accountWidgets[2] = new UserAccountWidget;
    accountWidgets[3] = new UserAccountWidget;

    for(auto &widget : accountWidgets) {
        connect(widget, &UserAccountWidget::validityChanged,
                this, &AccountPage::completeChanged);
        layout->addStretch();
        layout->addWidget(widget);
    }

    setLayout(layout);
}

bool AccountPage::isComplete() const {
    return std::all_of(accountWidgets.begin(), accountWidgets.end(),
                       [](UserAccountWidget *widget) {
        if(widget == nullptr) return true;
        return widget->isValid();
    });
}

#ifdef HAS_INSTALL_ENV
bool AccountPage::validatePage() {
    /* hack to re-initialise page in case the user goes back */
    horizonWizard()->removePage(HorizonWizard::Page_Commit);
    horizonWizard()->setPage(HorizonWizard::Page_Commit, new CommitPage);

    return true;
}
#endif  /* HAS_INSTALL_ENV */
