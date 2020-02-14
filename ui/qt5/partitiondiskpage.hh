/*
 * partitiondiskpage.hh - Definition of the UI.Partition.Install.UserPrompt page
 * horizon-qt5, the Qt 5 user interface for
 * Project Horizon
 *
 * Copyright (c) 2020 Adélie Linux and contributors.  All rights reserved.
 * This code is licensed under the AGPL 3.0 license, as noted in the
 * LICENSE-code file in the root directory of this repository.
 *
 * SPDX-License-Identifier: AGPL-3.0-only
 */

#ifndef PARTITIONDISKPAGE_HH
#define PARTITIONDISKPAGE_HH

#include "horizonwizardpage.hh"

#include <QListWidget>

class PartitionDiskPage : public HorizonWizardPage {
public:
    PartitionDiskPage(QWidget *parent = nullptr);
    void initializePage() override;
    bool isComplete() const override;
private:
    QListWidget *diskChooser;
};

#endif  /* !PARTITIONDISKPAGE_HH */
