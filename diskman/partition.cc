/*
 * partition.cc - Implementation of the Partition class
 * diskman, the Disk Manipulation library for
 * Project Horizon
 *
 * Copyright (c) 2020 Adélie Linux and contributors.  All rights reserved.
 * This code is licensed under the AGPL 3.0 license, as noted in the
 * LICENSE-code file in the root directory of this repository.
 *
 * SPDX-License-Identifier: AGPL-3.0-only
 */

#include "partition.hh"
#include "disk.hh"

#include <blkid/blkid.h>
#include <iostream>
#include <libfdisk/libfdisk.h>
#include <libudev.h>
#include <stdexcept>

namespace Horizon {
namespace DiskMan {

Partition::Partition(Disk &d, void *creation, int type) {
    switch(type) {
    case 0: { /* libfdisk */
        struct fdisk_partition *part = static_cast<struct fdisk_partition *>(creation);
        if(fdisk_partition_has_size(part)) {
            this->_size = fdisk_partition_get_size(part) * d.sector_size();
        } else {
            this->_size = 0;
        }
        char *name = fdisk_partname(d.node().c_str(),
                                    fdisk_partition_get_partno(part) + 1);
        this->_node = std::string(name);
        char *value;
        value = blkid_get_tag_value(nullptr, "TYPE", name);
        if(value != nullptr) {
            this->_fs_type = std::string(value);
            free(value);
        }
        value = blkid_get_tag_value(nullptr, "LABEL", name);
        if(value != nullptr) {
            this->_label = std::string(value);
            free(value);
        }
        free(name);
        break;
    }
    case 1: { /* udev */
        struct udev_device *dev = static_cast<struct udev_device *>(creation);
        const char *value;
        value = udev_device_get_property_value(dev, "ID_FS_TYPE");
        if(value != nullptr) this->_fs_type = std::string(value);
        value = udev_device_get_property_value(dev, "ID_FS_LABEL");
        if(value != nullptr) this->_label = std::string(value);
        value = udev_device_get_property_value(dev, "ID_PART_ENTRY_SIZE");
        if(value != nullptr) this->_size = strtoull(value, nullptr, 10) * 512;
        value = udev_device_get_property_value(dev, "DEVNAME");
        if(value != nullptr) this->_node = std::string(value);
        break;
    }
    default:
        throw std::invalid_argument{ "invalid type code" };
    }
}

}
}
