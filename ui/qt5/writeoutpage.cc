/*
 * writeoutpage.cc - Implementation of the UI.Writeout page
 * horizon-qt5, the Qt 5 user interface for
 * Project Horizon
 *
 * Copyright (c) 2019 Adélie Linux and contributors.  All rights reserved.
 * This code is licensed under the AGPL 3.0 license, as noted in the
 * LICENSE-code file in the root directory of this repository.
 *
 * SPDX-License-Identifier: AGPL-3.0-only
 */

#include "writeoutpage.hh"

WriteoutPage::WriteoutPage(QWidget *parent) : HorizonWizardPage(parent) {
    setTitle(tr("Save Installation Script"));
    loadWatermark("intro");
}
