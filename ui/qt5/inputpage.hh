/*
 * inputpage.hh - Definition of the UI.Input page
 * horizon-qt5, the Qt 5 user interface for
 * Project Horizon
 *
 * Copyright (c) 2019 Adélie Linux and contributors.  All rights reserved.
 * This code is licensed under the AGPL 3.0 license, as noted in the
 * LICENSE-code file in the root directory of this repository.
 *
 * SPDX-License-Identifier: AGPL-3.0-only
 */

#ifndef INPUTPAGE_HH
#define INPUTPAGE_HH

#include <QListWidget>

#include "horizonwizardpage.hh"

class InputPage : public HorizonWizardPage {
public:
    InputPage(QWidget *parent = nullptr);
    void initializePage() override;
private:
    QListWidget *layoutList;
};

#endif  /* !INPUTPAGE_HH */
