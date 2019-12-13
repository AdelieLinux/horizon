/*
 * pkgdefaults.hh - Definition of the UI.Packages.Choices page
 * horizon-qt5, the Qt 5 user interface for
 * Project Horizon
 *
 * Copyright (c) 2019 Adélie Linux and contributors.  All rights reserved.
 * This code is licensed under the AGPL 3.0 license, as noted in the
 * LICENSE-code file in the root directory of this repository.
 *
 * SPDX-License-Identifier: AGPL-3.0-only
 */

#ifndef PKGDEFAULTS_HH
#define PKGDEFAULTS_HH

#include "horizonwizardpage.hh"
#include <QRadioButton>

class PkgDefaultsPage : public HorizonWizardPage {
public:
    PkgDefaultsPage(QWidget *parent = nullptr);

    void initializePage();
private:
    QRadioButton *dashShell, *bashShell,
                 *s6Init, *sysvInit,
                 *eudev, *mdevd;
};

#endif  /* !PKGDEFAULTS_HH */
