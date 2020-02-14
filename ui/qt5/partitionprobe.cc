/*
 * partitionprobe.cc - Implementation of the disk probing thread routines
 * horizon-qt5, the Qt 5 user interface for
 * Project Horizon
 *
 * Copyright (c) 2019 Adélie Linux and contributors.  All rights reserved.
 * This code is licensed under the AGPL 3.0 license, as noted in the
 * LICENSE-code file in the root directory of this repository.
 *
 * SPDX-License-Identifier: AGPL-3.0-only
 */

#include "partitionprobe.hh"

void PartitionProbeThread::run() {
    vector<Disk> *disks = new vector<Disk>{myMan.find_disks(true, true, false)};
    sleep(1);
    emit foundDisks(disks);
}
