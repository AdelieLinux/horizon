/*
 * bootpage.hh - Definition of the UI.Boot page
 * horizon-qt5, the Qt 5 user interface for
 * Project Horizon
 *
 * Copyright (c) 2019 Adélie Linux and contributors.  All rights reserved.
 * This code is licensed under the AGPL 3.0 license, as noted in the
 * LICENSE-code file in the root directory of this repository.
 *
 * SPDX-License-Identifier: AGPL-3.0-only
 */

#ifndef BOOTPAGE_HH
#define BOOTPAGE_HH

#include "horizonwizardpage.hh"

#include <QRadioButton>

class BootPage : public HorizonWizardPage {
public:
    BootPage(QWidget *parent = nullptr);

    void initializePage();
private:
    QRadioButton *yesGrub, *noGrub,
                 *easyKernel, *mlKernel;
};

#endif  /* !BOOTPAGE_HH */
