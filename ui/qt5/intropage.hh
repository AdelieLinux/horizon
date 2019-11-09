/*
 * intropage.hh - Definition of the UI.Intro page
 * horizon-qt5, the Qt 5 user interface for
 * Project Horizon
 *
 * Copyright (c) 2019 Adélie Linux and contributors.  All rights reserved.
 * This code is licensed under the AGPL 3.0 license, as noted in the
 * LICENSE-code file in the root directory of this repository.
 *
 * SPDX-License-Identifier: AGPL-3.0-only
 */

#ifndef INTROPAGE_HH
#define INTROPAGE_HH

#ifdef HAS_INSTALL_ENV
#include <QPushButton>
#endif  /* HAS_INSTALL_ENV */

#include "horizonwizardpage.hh"

class IntroPage : public HorizonWizardPage {
public:
    IntroPage(QWidget *parent = nullptr);

#ifdef HAS_INSTALL_ENV
private:
    QPushButton *toolButton;
#endif  /* HAS_INSTALL_ENV */
};

#endif  /* !INTROPAGE_HH */
