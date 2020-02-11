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
#include <libudev.h>

namespace Horizon {
namespace DiskMan {

/*! The full serial number of the disk */
std::string _full_serial;

#define SAFE_SET(ivar, udev_call) \
    value = udev_call;\
    if(value != nullptr) {\
        ivar = std::string(value);\
    }

Disk::Disk(void *creation, bool partition) {
    struct udev_device *device = static_cast<struct udev_device *>(creation);
    const char *value;

    SAFE_SET(_name, udev_device_get_sysname(device));
    SAFE_SET(_model, udev_device_get_property_value(device, "ID_MODEL"));
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
}

}
}
