/*
 * partitionprobe.hh - Definition of the disk probing thread routines
 * horizon-qt5, the Qt 5 user interface for
 * Project Horizon
 *
 * Copyright (c) 2019 Adélie Linux and contributors.  All rights reserved.
 * This code is licensed under the AGPL 3.0 license, as noted in the
 * LICENSE-code file in the root directory of this repository.
 *
 * SPDX-License-Identifier: AGPL-3.0-only
 */

#ifndef PARTITIONPROBE_HH
#define PARTITIONPROBE_HH

#include <QThread>
#include <diskman/diskman.hh>

using std::vector;
using Horizon::DiskMan::Disk;

class PartitionProbeThread : public QThread {
    Q_OBJECT
public:
    void run() override;
signals:
    void foundDisks(void *disks);
private:
    Horizon::DiskMan::DiskMan myMan;
};

#endif  /* !PARTITIONPROBE_HH */
