/*
 * finishedpage.cc - Implementation of the UI.Finish page
 * horizon-run-qt5, the Qt 5 executor user interface for
 * Project Horizon
 *
 * Copyright (c) 2019 Adélie Linux and contributors.  All rights reserved.
 * This code is licensed under the AGPL 3.0 license, as noted in the
 * LICENSE-code file in the root directory of this repository.
 *
 * SPDX-License-Identifier: AGPL-3.0-only
 */

#include "finishedpage.hh"

FinishedPage::FinishedPage(QWidget *parent) : QWizardPage(parent) {
    setTitle(tr("Adélie Linux Successfully Installed"));
}
