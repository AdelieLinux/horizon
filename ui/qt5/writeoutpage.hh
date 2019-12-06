/*
 * writeoutpage.hh - Definition of the UI.Writeout page
 * horizon-qt5, the Qt 5 user interface for
 * Project Horizon
 *
 * Copyright (c) 2019 Adélie Linux and contributors.  All rights reserved.
 * This code is licensed under the AGPL 3.0 license, as noted in the
 * LICENSE-code file in the root directory of this repository.
 *
 * SPDX-License-Identifier: AGPL-3.0-only
 */

#ifndef WRITEOUTPAGE_HH
#define WRITEOUTPAGE_HH

#include "horizonwizardpage.hh"

class WriteoutPage : public HorizonWizardPage {
public:
    WriteoutPage(QWidget *parent = nullptr);
};

#endif  /* !WRITEOUTPAGE_HH */
