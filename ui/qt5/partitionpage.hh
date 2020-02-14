/*
 * partitionpage.hh - Definition of the UI.Partition page:
 * either UI.Partition.Runtime.DiskDetails, or UI.Partition.Install.Details
 * horizon-qt5, the Qt 5 user interface for
 * Project Horizon
 *
 * Copyright (c) 2020 Adélie Linux and contributors.  All rights reserved.
 * This code is licensed under the AGPL 3.0 license, as noted in the
 * LICENSE-code file in the root directory of this repository.
 *
 * SPDX-License-Identifier: AGPL-3.0-only
 */

#ifndef PARTITIONPAGE_HH
#define PARTITIONPAGE_HH

#ifdef HAS_INSTALL_ENV
#   include <QLabel>
#   include <QProgressBar>
#   include <QPushButton>
#   include "partitionprobe.hh"
#endif  /* HAS_INSTALL_ENV */


#include "horizonwizardpage.hh"

class PartitionPage : public HorizonWizardPage {
public:
    PartitionPage(QWidget *parent = nullptr);
    void initializePage() override;
    bool isComplete() const override;
private:
#ifdef HAS_INSTALL_ENV
    void scanDisks();
    void processDisks(void *disks);
    bool scanDone;

    QProgressBar *progress;
    QLabel *descLabel;
    QPushButton *scanButton;
    PartitionProbeThread *thread;
#endif  /* HAS_INSTALL_ENV */
};

#endif  /* !PARTITIONPAGE_HH */
