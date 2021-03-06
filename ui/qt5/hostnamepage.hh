/*
 * hostnamepage.hh - Definition of the UI.SysMeta.Hostname page
 * horizon-qt5, the Qt 5 user interface for
 * Project Horizon
 *
 * Copyright (c) 2019 Adélie Linux and contributors.  All rights reserved.
 * This code is licensed under the AGPL 3.0 license, as noted in the
 * LICENSE-code file in the root directory of this repository.
 *
 * SPDX-License-Identifier: AGPL-3.0-only
 */

#ifndef HOSTNAMEPAGE_HH
#define HOSTNAMEPAGE_HH

#include "horizonwizardpage.hh"

#include <QLineEdit>

class HostnamePage : public HorizonWizardPage {
public:
    HostnamePage(QWidget *parent = nullptr);

    void initializePage();
private:
    QLineEdit *hostnameEdit;
};

#endif  /* !HOSTNAMEPAGE_HH */
