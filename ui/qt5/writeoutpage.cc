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

#include <QLabel>
#include <QVBoxLayout>

WriteoutPage::WriteoutPage(QWidget *parent) : HorizonWizardPage(parent) {
    setTitle(tr("Save Installation Script"));
    loadWatermark("intro");

    QLabel *descLabel = new QLabel(tr("You have provided all the necessary information for System Installation.  "
        "Choose Save to write your installfile to disk and close System Installation.\n\n"
        "For more information on how to use the installation script you've generated, review the Adélie Linux Installation Handbook.\n\n"
        "You may also customise the installation script by editing it using a text editor.  "
        "Review the HorizonScript Reference for more information on available options."));
    descLabel->setWordWrap(true);
    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(descLabel);
    setLayout(layout);
}
