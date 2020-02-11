/*
 * diskman.hh - Public interface for the DiskMan library
 * diskman, the Disk Manipulation library for
 * Project Horizon
 *
 * Copyright (c) 2020 Adélie Linux and contributors.  All rights reserved.
 * This code is licensed under the AGPL 3.0 license, as noted in the
 * LICENSE-code file in the root directory of this repository.
 *
 * SPDX-License-Identifier: AGPL-3.0-only
 */

#ifndef DISKMAN__DISKMAN_HH
#define DISKMAN__DISKMAN_HH

#include <memory>
#include <vector>

#include "disk.hh"

namespace Horizon {
namespace DiskMan {

class DiskMan {
    struct impl;
    std::unique_ptr<impl> pImpl;
public:
    DiskMan();
    ~DiskMan();

    /*! Find all available disks on this system.
     * @param include_part      Include partitions with disks.
     * @param include_vg        Include LVM volume groups as disks.
     * @param include_lvm       Include LVM volumes as disks.
     */
    std::vector<Disk> find_disks(bool include_part = true,
                                 bool include_vg = true,
                                 bool include_lvm = false);
};

}
}

#endif  /* !DISKMAN__DISKMAN_HH */
