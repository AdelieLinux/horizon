/*
 * partitionpage.cc - Implementation of the UI.Partition page:
 * either UI.Partition.Runtime.DiskDetails, or UI.Partition.Install.Details
 * horizon-qt5, the Qt 5 user interface for
 * Project Horizon
 *
 * Copyright (c) 2019 Adélie Linux and contributors.  All rights reserved.
 * This code is licensed under the AGPL 3.0 license, as noted in the
 * LICENSE-code file in the root directory of this repository.
 *
 * SPDX-License-Identifier: AGPL-3.0-only
 */

#include "partitionpage.hh"

#include <QVBoxLayout>

PartitionPage::PartitionPage(QWidget *parent) : HorizonWizardPage(parent) {
    loadWatermark("disk");
#ifdef HAS_INSTALL_ENV
    setTitle(tr("Detecting Disks"));

    thread = nullptr;
    scanDone = false;

    descLabel = new QLabel;
    descLabel->setWordWrap(true);
    progress = new QProgressBar;
    progress->setRange(0, 0);

    scanButton = new QPushButton(tr("Rescan Devices"));
    connect(scanButton, &QPushButton::clicked, this, &PartitionPage::scanDisks);
    scanButton->setHidden(true);

    QVBoxLayout *layout = new QVBoxLayout;
    layout->addStretch();
    layout->addWidget(descLabel);
    layout->addWidget(progress);
    layout->addWidget(scanButton, 0, Qt::AlignCenter);
    layout->addStretch();

    setLayout(layout);
#else  /* !HAS_INSTALL_ENV */
    setTitle(tr("Enter Disk Information"));
#endif  /* HAS_INSTALL_ENV */
}

void PartitionPage::initializePage() {
#ifdef HAS_INSTALL_ENV
    scanDisks();
#endif
}

bool PartitionPage::isComplete() const {
#ifdef HAS_INSTALL_ENV
    return scanDone;
#else  /* !HAS_INSTALL_ENV */
    // determine whether a valid disk has been input
    return false;
#endif  /* HAS_INSTALL_ENV */
}

#ifdef HAS_INSTALL_ENV
void PartitionPage::scanDisks() {
    descLabel->setText(tr("Please wait while System Installation collects information on your computer's disk drives."));
    scanButton->setEnabled(false);
    scanButton->setHidden(true);
    progress->setHidden(false);

    if(thread != nullptr) {
        thread->deleteLater();
    }

    thread = new PartitionProbeThread;
    connect(thread, &PartitionProbeThread::foundDisks,
            this, &PartitionPage::processDisks);
    thread->start();
}

void PartitionPage::processDisks(void *disk_obj) {
    /* Qt can only fling void*s around threads; convert back to vec of Disk */
    vector<Disk> *disks = static_cast<vector<Disk> *>(disk_obj);

    scanButton->setEnabled(true);
    scanButton->setHidden(false);
    progress->setHidden(true);
    descLabel->setText(tr("System Installation has finished scanning your computer for disk drives.\n\nIf you've changed your computer's disk drive layout, choose Rescan Devices to run another scan."));

    /* Copy our disk information into the wizard's global object */
    horizonWizard()->disks.swap(*disks);
    delete disks;

    scanDone = true;
    emit completeChanged();
    wizard()->next();
}
#endif