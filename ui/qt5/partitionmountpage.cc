/*
 * partitionmountpage.cc - Implementation of UI.Partition.Install.Mount page
 * horizon-qt5, the Qt 5 user interface for
 * Project Horizon
 *
 * Copyright (c) 2020 Adélie Linux and contributors.  All rights reserved.
 * This code is licensed under the AGPL 3.0 license, as noted in the
 * LICENSE-code file in the root directory of this repository.
 *
 * SPDX-License-Identifier: AGPL-3.0-only
 */

#include "partitionmountpage.hh"

PartitionMountPage::PartitionMountPage(QWidget *parent)
    : HorizonWizardPage(parent) {
    loadWatermark("disk");
    setTitle(tr("Set Mount Points"));
}
