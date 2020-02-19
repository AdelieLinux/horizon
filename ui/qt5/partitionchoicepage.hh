/*
 * partitionchoicepage.hh - Definition of UI.Partition.Install.UserPrompt page
 * horizon-qt5, the Qt 5 user interface for
 * Project Horizon
 *
 * Copyright (c) 2020 Adélie Linux and contributors.  All rights reserved.
 * This code is licensed under the AGPL 3.0 license, as noted in the
 * LICENSE-code file in the root directory of this repository.
 *
 * SPDX-License-Identifier: AGPL-3.0-only
 */

#ifndef PARTITIONCHOICEPAGE_HH
#define PARTITIONCHOICEPAGE_HH

#include "horizonwizardpage.hh"

#include <QButtonGroup>
#include <QLabel>
#include <QRadioButton>

class PartitionChoicePage : public HorizonWizardPage {
public:
    PartitionChoicePage(QWidget *parent = nullptr);
    void initializePage() override;
    bool isComplete() const override;
    int nextId() const override;
    bool validatePage() override;
private:
    QLabel *descLabel;
    QButtonGroup *buttons;
    QRadioButton *eraseButton;
    QLabel *eraseLabel;
    QRadioButton *fitInButton;
    QLabel *fitInLabel;
    QRadioButton *manualButton;
    QLabel *manualLabel;
    QRadioButton *useExistingButton;
    QLabel *useExistingLabel;
};

#endif  /* !PARTITIONCHOICEPAGE_HH */
