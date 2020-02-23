/*
 * networkingpage.hh - Definition of the UI.Network.AddressType page
 * horizon-qt5, the Qt 5 user interface for
 * Project Horizon
 *
 * Copyright (c) 2019 Adélie Linux and contributors.  All rights reserved.
 * This code is licensed under the AGPL 3.0 license, as noted in the
 * LICENSE-code file in the root directory of this repository.
 *
 * SPDX-License-Identifier: AGPL-3.0-only
 */

#ifndef NETWORKINGPAGE_HH
#define NETWORKINGPAGE_HH

#include "horizonwizardpage.hh"

#include <QButtonGroup>
#include <QRadioButton>

class NetworkingPage : public HorizonWizardPage {
public:
    NetworkingPage(QWidget *parent = nullptr);

    void initializePage() override;
    bool isComplete() const override;
    int nextId() const override;
private:
    QButtonGroup *radioGroup;
    QRadioButton *simple, *advanced, *skip;
};

#endif /* !NETWORKINGPAGE_HH */
