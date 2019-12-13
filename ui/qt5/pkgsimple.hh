/*
 * pkgsimple.hh - Definition of the UI.Packages.SimpleSel page
 * horizon-qt5, the Qt 5 user interface for
 * Project Horizon
 *
 * Copyright (c) 2019 Adélie Linux and contributors.  All rights reserved.
 * This code is licensed under the AGPL 3.0 license, as noted in the
 * LICENSE-code file in the root directory of this repository.
 *
 * SPDX-License-Identifier: AGPL-3.0-only
 */

#ifndef PKGSIMPLE_HH
#define PKGSIMPLE_HH

#include "horizonwizardpage.hh"

#include <QRadioButton>

class PkgSimplePage : public HorizonWizardPage {
public:
    PkgSimplePage(QWidget *parent = nullptr);
    void initializePage();
    int nextId() const;
    bool validatePage();
private:
    QRadioButton *standardButton;
};

#endif  /* !PKGSIMPLE_HH */
