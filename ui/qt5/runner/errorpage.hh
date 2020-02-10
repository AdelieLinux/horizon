/*
 * errorpage.hh - Definition of the UI.Perform.Error page
 * horizon-run-qt5, the Qt 5 executor user interface for
 * Project Horizon
 *
 * Copyright (c) 2019 Adélie Linux and contributors.  All rights reserved.
 * This code is licensed under the AGPL 3.0 license, as noted in the
 * LICENSE-code file in the root directory of this repository.
 *
 * SPDX-License-Identifier: AGPL-3.0-only
 */

#ifndef ERRORPAGE_HH
#define ERRORPAGE_HH

#include "../horizonwizardpage.hh"
#include <QLabel>

class ErrorPage : public HorizonWizardPage {
public:
    ErrorPage(QWidget *parent = nullptr);
    void initializePage();
private:
    QLabel *descLabel;
    QLabel *deetLabel;
};

#endif  /* !ERRORPAGE_HH */
