/*
 * disk.cc - Implementation of the Disk class
 * diskman, the Disk Manipulation library for
 * Project Horizon
 *
 * Copyright (c) 2020 Adélie Linux and contributors.  All rights reserved.
 * This code is licensed under the AGPL 3.0 license, as noted in the
 * LICENSE-code file in the root directory of this repository.
 *
 * SPDX-License-Identifier: AGPL-3.0-only
 */

#include "disk.hh"

#include <cstring>
#include <iostream>
#include <libfdisk/libfdisk.h>
#include <libudev.h>
#include <stdexcept>

namespace Horizon {
namespace DiskMan {

#define SAFE_SET(ivar, udev_call) \
    value = udev_call;\
    if(value != nullptr) {\
        ivar = std::string(value);\
    }

Disk::Disk(void *creation, int type, bool partition) {
    switch(type) {
    case 0: { /* udev */
        struct udev_device *device = static_cast<struct udev_device *>(creation);
        const char *value;

        SAFE_SET(_name, udev_device_get_sysname(device));
        SAFE_SET(_model, udev_device_get_property_value(device, "ID_MODEL"));
        SAFE_SET(_full_serial, udev_device_get_property_value(device, "ID_SERIAL"));
        SAFE_SET(_node, udev_device_get_devnode(device));
        SAFE_SET(_devpath, udev_device_get_devpath(device));

        value = udev_device_get_property_value(device, "ID_PART_TABLE_TYPE");
        if(value == nullptr) {
            _has_label = false;
            _label = Unknown;
        } else {
            _has_label = true;
            if(::strcmp(value, "apm") == 0) {
                _label = APM;
            } else if(::strcmp(value, "dos") == 0) {
                _label = MBR;
            } else if(::strcmp(value, "gpt") == 0) {
                _label = GPT;
            } else {
                _label = Unknown;
            }
        }

        value = udev_device_get_property_value(device, "ID_FS_TYPE");
        if(value == nullptr) {
            _has_fs = false;
        } else {
            _has_fs = true;
            _fs_type = std::string(value);
            SAFE_SET(_fs_label, udev_device_get_property_value(device, "ID_FS_LABEL"));
        }
        break;
    }
    default:
        throw new std::invalid_argument{ "invalid type code" };
    }

    total_mb = free_mb = contiguous_mb = 0;

    struct fdisk_context *ctxt = fdisk_new_context();
    if(ctxt != nullptr) {
        /* Open the device in read-only mode.  We don't need to write to it */
        if(fdisk_assign_device(ctxt, _node.c_str(), 1) == 0) {
            _sector = fdisk_get_sector_size(ctxt);
            total_mb = (fdisk_get_nsectors(ctxt) * _sector) / 1048576;
            struct fdisk_table *frees = nullptr;
            if(fdisk_has_label(ctxt) != 1) {
                /* Disk has no label, so consider it empty */
                free_mb = contiguous_mb = total_mb;
            } else if(fdisk_get_freespaces(ctxt, &frees) == 0) {
                for(size_t next = 0; next < fdisk_table_get_nents(frees);
                    next++) {
                    /* Each entry in frees is a "free space partition". */
                    struct fdisk_partition *part =
                            fdisk_table_get_partition(frees, next);
                    fdisk_sector_t size;
                    if(!fdisk_partition_has_size(part)) continue;
                    size = (fdisk_partition_get_size(part) * _sector) / 1048576;
                    free_mb += size;
                    if(size > contiguous_mb) contiguous_mb = size;
                }
                fdisk_unref_table(frees);
            }
            /* We used to free ctxt here, but it's useful for partition probing */
        } else {
            fdisk_unref_context(ctxt);
            ctxt = nullptr;
        }
    }

    if(partition) {
        if(reload_partitions()) {
            /* We're good */
        } else if(type == 0) {
            /* fallback to udev, if available */
            std::cerr << "Falling back to udev partition probing" << std::endl;

            struct udev_device *device = static_cast<struct udev_device *>(creation);
            struct udev *udev = udev_device_get_udev(device);
            struct udev_enumerate *part_enum = udev_enumerate_new(udev);
            if(part_enum != NULL) {
                struct udev_list_entry *first;

                udev_enumerate_add_match_subsystem(part_enum, "block");
                udev_enumerate_add_match_property(part_enum, "DEVTYPE",
                                                  "partition");
                udev_enumerate_add_match_parent(part_enum, device);
                udev_enumerate_scan_devices(part_enum);

                first = udev_enumerate_get_list_entry(part_enum);
                if(first != NULL) {
                    struct udev_list_entry *item;
                    udev_list_entry_foreach(item, first) {
                        const char *path = udev_list_entry_get_name(item);
                        struct udev_device *part_device = udev_device_new_from_syspath(udev, path);
                        if(part_device != nullptr) {
                            _partitions.push_back(Partition(*this, part_device, 1));
                            udev_device_unref(part_device);
                        }
                    }
                }
                udev_enumerate_unref(part_enum);
            }
        } else {
            std::cerr << "Cannot load partitions for " << _name << std::endl;
        }
    }

    if(ctxt != nullptr) {
        fdisk_unref_context(ctxt);
    }
}

bool Disk::reload_partitions() {
    bool success = false;
    struct fdisk_context *ctxt = fdisk_new_context();
    struct fdisk_table *parts = nullptr;

    if(ctxt == nullptr) {
        return false;
    }

    /* Open the device in read-only mode.  We don't need to write to it */
    if(fdisk_assign_device(ctxt, _node.c_str(), 1) != 0) {
        goto destroy_context;
    }

    if(fdisk_get_partitions(ctxt, &parts) == 0) {
        _partitions.clear();
        for(size_t next = 0; next < fdisk_table_get_nents(parts);
            next++) {
            struct fdisk_partition *part =
                    fdisk_table_get_partition(parts, next);
            _partitions.push_back(Partition(*this, part, 0));
        }
        success = true;
        fdisk_unref_table(parts);
    }

destroy_context:
    fdisk_unref_context(ctxt);
    return success;
}

const std::vector<Partition> Disk::partitions() const {
    if(!this->has_label()) {
        throw std::logic_error{ "attempt to retrieve partitions for non-labelled disk" };
    }

    return this->_partitions;
}

}
}
