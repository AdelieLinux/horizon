/*
 * horizonwizardpage.hh - Definition of our custom QWizardPage subclass
 * horizon-qt5, the Qt 5 user interface for
 * Project Horizon
 *
 * Copyright (c) 2019 Adélie Linux and contributors.  All rights reserved.
 * This code is licensed under the AGPL 3.0 license, as noted in the
 * LICENSE-code file in the root directory of this repository.
 *
 * SPDX-License-Identifier: AGPL-3.0-only
 */

#ifndef HORIZONWIZARDPAGE_HH
#define HORIZONWIZARDPAGE_HH

#include <QWizardPage>
#include <string>

#include "horizonwizard.hh"

class HorizonWizardPage : public QWizardPage {
public:
    HorizonWizardPage(QWidget *parent = 0) : QWizardPage(parent) {}

    void loadWatermark(std::string page);
    HorizonWizard *horizonWizard() const;
};

#endif  /* !HORIZONWIZARDPAGE_HH */
