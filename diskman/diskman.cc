/*
 * diskman.cc - Common routines for the DiskMan library
 * diskman, the Disk Manipulation library for
 * Project Horizon
 *
 * Copyright (c) 2020 Adélie Linux and contributors.  All rights reserved.
 * This code is licensed under the AGPL 3.0 license, as noted in the
 * LICENSE-code file in the root directory of this repository.
 *
 * SPDX-License-Identifier: AGPL-3.0-only
 */

#include "diskman.hh"

#include <iostream>
#include <libudev.h>

namespace Horizon {
namespace DiskMan {

struct DiskMan::impl {
    struct udev *udev;

    impl() {
        udev = udev_new();
    }
    ~impl() {
        udev_unref(udev);
    }
    impl(const impl &other) {
        this->udev = udev_ref(other.udev);
    }
};

DiskMan::DiskMan() : pImpl {std::make_unique<impl>()} {
}

DiskMan::~DiskMan() = default;

std::vector<Disk> DiskMan::find_disks(bool include_part, bool include_vg,
                                      bool include_lvm) {
    struct udev_enumerate *disk_enum = udev_enumerate_new(pImpl->udev);
    struct udev_list_entry *first, *item;
    struct udev_device *device = nullptr;
    std::vector<Disk> disks;

    if(disk_enum == nullptr) {
        std::cerr << "Couldn't connect to udev" << std::endl;
        return {};
    }
    udev_enumerate_add_match_subsystem(disk_enum, "block");
    udev_enumerate_add_match_property(disk_enum, "DEVTYPE", "disk");
    udev_enumerate_scan_devices(disk_enum);
    first = udev_enumerate_get_list_entry(disk_enum);
    if(first == nullptr) {
        std::cerr << "No block devices found" << std::endl;
        return {};
    }

    udev_list_entry_foreach(item, first) {
        const char *path = udev_list_entry_get_name(item);
        if(device != nullptr) udev_device_unref(device);
        device = udev_device_new_from_syspath(pImpl->udev, path);
        std::string name(udev_device_get_sysname(device));
        if(name.compare(0, 4, "loop") == 0
                || name.compare(0, 3, "ram") == 0) {
            /* Don't include loop or ram devices */
            continue;
        }
        if(!include_lvm && name.compare(0, 3, "dm-") == 0) {
            /* Skip LVM volumes if requested. */
            continue;
        }
        if(udev_device_get_property_value(device, "ID_CDROM") != nullptr) {
            /* REQ: UI.Partition.Install.Ignore */
            continue;
        }
        disks.push_back(Disk(device, 0, include_part));
    }

    if(device != nullptr) {
        udev_device_unref(device);
    }
    udev_enumerate_unref(disk_enum);

    return disks;
}

}
}
