/*
 * partitionmountpage.hh - Definition of the UI.Partition.Install.Mount page
 * horizon-qt5, the Qt 5 user interface for
 * Project Horizon
 *
 * Copyright (c) 2020 Adélie Linux and contributors.  All rights reserved.
 * This code is licensed under the AGPL 3.0 license, as noted in the
 * LICENSE-code file in the root directory of this repository.
 *
 * SPDX-License-Identifier: AGPL-3.0-only
 */

#ifndef PARTITIONMOUNTPAGE_HH
#define PARTITIONMOUNTPAGE_HH

#include "horizonwizardpage.hh"
#include <QListWidget>
#include <QPushButton>

class PartitionMountPage : public HorizonWizardPage {
public:
    PartitionMountPage(QWidget *parent = nullptr);

    bool isComplete() const override;
    QStringList mountLines() const;
private:
    QListWidget *mountList;
    QPushButton *addMountButton, *delMountButton;
#ifdef HAS_INSTALL_ENV
    QPushButton *rescanButton;
#endif  /* !HAS_INSTALL_ENV */
};

#endif  /* !PARTITIONMOUNTPAGE_HH */
