/*
 * firmwarepage.hh - Definition of the UI.Firmware page
 * horizon-qt5, the Qt 5 user interface for
 * Project Horizon
 *
 * Copyright (c) 2019 Adélie Linux and contributors.  All rights reserved.
 * This code is licensed under the AGPL 3.0 license, as noted in the
 * LICENSE-code file in the root directory of this repository.
 *
 * SPDX-License-Identifier: AGPL-3.0-only
 */

#ifndef FIRMWAREPAGE_HH
#define FIRMWAREPAGE_HH

#include "horizonwizardpage.hh"

#include <QButtonGroup>
#include <QRadioButton>

class FirmwarePage : public HorizonWizardPage {
public:
    FirmwarePage(QWidget *parent = nullptr);
private:
    QButtonGroup *firmwareChoice;
    QRadioButton *yesButton;
    QRadioButton *noButton;
};

#endif  /* !FIRMWAREPAGE_HH */
