/*
 * pkgcustom.hh - Definition of the UI.Packages.Custom page
 * horizon-qt5, the Qt 5 user interface for
 * Project Horizon
 *
 * Copyright (c) 2019 Adélie Linux and contributors.  All rights reserved.
 * This code is licensed under the AGPL 3.0 license, as noted in the
 * LICENSE-code file in the root directory of this repository.
 *
 * SPDX-License-Identifier: AGPL-3.0-only
 */

#ifndef PKGCUSTOM_HH
#define PKGCUSTOM_HH

#include "horizonwizardpage.hh"
#include "pkgmodel.hh"
#include <QLabel>

class PkgCustomPage : public HorizonWizardPage {
public:
    PkgCustomPage(QWidget *parent = nullptr);
    void initializePage();
private:
    QLabel *pkgDescLabel;
    PkgItemModel *model;
};

#endif  /* !PKGCUSTOM_HH */
