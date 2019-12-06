/*
 * commitpage.cc - Definition of the UI.Commit page
 * horizon-qt5, the Qt 5 user interface for
 * Project Horizon
 *
 * Copyright (c) 2019 Adélie Linux and contributors.  All rights reserved.
 * This code is licensed under the AGPL 3.0 license, as noted in the
 * LICENSE-code file in the root directory of this repository.
 *
 * SPDX-License-Identifier: AGPL-3.0-only
 */

#ifndef COMMITPAGE_HH
#define COMMITPAGE_HH

#include "horizonwizardpage.hh"

#include <QLabel>

class CommitPage : public HorizonWizardPage {
public:
    CommitPage(QWidget *parent = nullptr);

    void initializePage();
private:
    QLabel *choices;
};

#endif  /* !COMMITPAGE_HH */
