/*
 * disk.cc - Implementation of the Key classes for disk manipulation
 * libhscript, the HorizonScript library for
 * Project Horizon
 *
 * Copyright (c) 2019 Adélie Linux and contributors.  All rights reserved.
 * This code is licensed under the AGPL 3.0 license, as noted in the
 * LICENSE-code file in the root directory of this repository.
 *
 * SPDX-License-Identifier: AGPL-3.0-only
 */

#include <algorithm>
#include <cstring>          /* strerror */
#include <fstream>
#include <string>
#ifdef HAS_INSTALL_ENV
#   include <assert.h>         /* assert */
#   include <blkid/blkid.h>    /* blkid_get_tag_value */
#   include <libudev.h>        /* udev_* */
#   include <sys/mount.h>      /* mount */
#   include <sys/stat.h>       /* mkdir, stat */
#   include <sys/types.h>      /* S_* */
#   include <unistd.h>         /* access */
#endif /* HAS_INSTALL_ENV */
#include "disk.hh"
#include "util/output.hh"

using namespace Horizon::Keys;

Key *DiskId::parseFromData(const std::string &data, int lineno, int *errors,
                           int *warnings) {
    std::string block, ident;
    std::string::size_type block_end = data.find_first_of(' ');
    if(block_end == std::string::npos) {
        if(errors) *errors += 1;
        output_error("installfile:" + std::to_string(lineno),
                     "diskid: expected an identification string",
                     "valid format for diskid is: [block] [id-string]");
        return nullptr;
    }

    block = data.substr(0, block_end);
    ident = data.substr(block_end + 1);
    return new DiskId(lineno, block, ident);
}

bool DiskId::validate(ScriptOptions options) const {
    /* We only validate if running in an Installation Environment. */
    if(!options.test(InstallEnvironment)) return true;

#ifdef HAS_INSTALL_ENV
    /* Unlike 'mount', 'diskid' *does* require that the block device exist
     * before installation begins.  This test is always valid. */
    struct stat blk_stat;
    const char *block_c = _block.c_str();
    if(access(block_c, F_OK) != 0 || stat(block_c, &blk_stat) != 0) {
        output_error("installfile:" + std::to_string(line),
                     "diskid: error opening device " + _block,
                     strerror(errno));
        return false;
    }
    if(!S_ISBLK(blk_stat.st_mode)) {
        output_error("installfile:" + std::to_string(line),
                     "diskid: " + _block + " is not a valid block device");
        return false;
    }
#endif /* HAS_INSTALL_ENV */
    return true;
}

bool DiskId::execute(ScriptOptions options) const {
    bool match = false;
    if(!options.test(InstallEnvironment)) return true;

#ifdef HAS_INSTALL_ENV
    struct udev *udev;
    struct udev_device *device;
    const char *serial;
    struct stat blk_stat;
    const char *block_c = _block.c_str();
    if(stat(block_c, &blk_stat) != 0) {
        output_error("installfile:" + std::to_string(line),
                     "diskid: error opening device " + _block,
                     strerror(errno));
        return false;
    }
    assert(S_ISBLK(blk_stat.st_mode));

    udev = udev_new();
    if(!udev) {
        output_error("installfile:" + std::to_string(line),
                     "diskid: failed to communicate with udevd",
                     "cannot read disk information");
        return false;
    }
    device = udev_device_new_from_devnum(udev, 'b', blk_stat.st_rdev);
    if(!device) {
        udev_unref(udev);
        output_error("installfile:" + std::to_string(line),
                     "diskid: failed to retrieve disk from udevd",
                     "cannot read disk information");
        return false;
    }

    serial = udev_device_get_property_value(device, "ID_SERIAL");
    /* If we can't get the serial for this device, it's not a disk */
    if(serial) {
        std::string full_str(serial);
        match = (full_str.find(_ident) != std::string::npos);
    }

    udev_device_unref(device);
    udev_unref(udev);
#endif /* HAS_INSTALL_ENV */

    return match;
}

Key *Mount::parseFromData(const std::string &data, int lineno, int *errors,
                          int *warnings) {
    std::string dev, where, opt;
    std::string::size_type where_pos, opt_pos;
    bool any_failure = false;

    long spaces = std::count(data.cbegin(), data.cend(), ' ');
    if(spaces < 1 || spaces > 2) {
        if(errors) *errors += 1;
        /* Don't bother with any_failure, because this is immediately fatal. */
        output_error("installfile:" + std::to_string(lineno),
                     "mount: expected either 2 or 3 elements, got: " +
                     std::to_string(spaces), "");
        return nullptr;
    }

    where_pos = data.find_first_of(' ');
    opt_pos = data.find_first_of(' ', where_pos + 1);

    dev = data.substr(0, where_pos);
    where = data.substr(where_pos + 1, (opt_pos - where_pos - 1));
    if(opt_pos != std::string::npos && data.length() > opt_pos + 1) {
        opt = data.substr(opt_pos + 1);
    }

    if(dev.compare(0, 4, "/dev")) {
        if(errors) *errors += 1;
        any_failure = true;
        output_error("installfile:" + std::to_string(lineno),
                     "mount: element 1: expected device node",
                     "'" + dev + "' is not a valid device node");
    }

    if(where[0] != '/') {
        if(errors) *errors += 1;
        any_failure = true;
        output_error("installfile:" + std::to_string(lineno),
                     "mount: element 2: expected absolute path",
                     "'" + where + "' is not a valid absolute path");
    }

    if(any_failure) return nullptr;

    return new Mount(lineno, dev, where, opt);
}

bool Mount::validate(ScriptOptions options) const {
    /* We only validate if running in an Installation Environment. */
    if(!options.test(InstallEnvironment)) return true;

#ifdef HAS_INSTALL_ENV
    /* XXX TODO: This will fail validation if the block device does not
     * already exist.  However, we must take in to account that block devices
     * may not yet exist during the script validation phase.  This check may
     * need to happen in Script::validate like the Uniqueness tests. */
    return(access(this->device().c_str(), F_OK) == 0);
#endif /* HAS_INSTALL_ENV */
}

bool Mount::execute(ScriptOptions options) const {
    const std::string actual_mount = "/target" + this->mountpoint();
    const char *fstype = nullptr;

    /* We have to get the filesystem for the node. */
    if(options.test(Simulate)) {
        fstype = "auto";
    }
#ifdef HAS_INSTALL_ENV
    else {
        fstype = blkid_get_tag_value(nullptr, "TYPE", this->device().c_str());
        if(fstype == nullptr) {
            output_error("installfile:" + std::to_string(this->lineno()),
                         "mount: cannot determine filesystem type for device",
                         this->device());
            return false;
        }
    }
#endif /* HAS_INSTALL_ENV */

    output_info("installfile:" + std::to_string(this->lineno()),
                "mount: mounting " + this->device() + " on " +
                this->mountpoint());
    if(options.test(Simulate)) {
        std::cout << "mount ";
        if(!this->options().empty()) {
            std::cout << "-o " << this->options() << " ";
        }
        std::cout << this->device() << " " << actual_mount << std::endl;
    }
#ifdef HAS_INSTALL_ENV
    else {
        /* mount */
        if(mount(this->device().c_str(), actual_mount.c_str(), fstype, 0,
                 this->options().c_str()) != 0) {
            output_warning("installfile:" + std::to_string(this->lineno()),
                           "mount: error mounting " + this->mountpoint() +
                           "with options; retrying without", strerror(errno));
            if(mount(this->device().c_str(), actual_mount.c_str(), fstype, 0,
                     nullptr) != 0) {
                output_error("installfile:" + std::to_string(this->lineno()),
                             "mount: error mounting " + this->mountpoint() +
                             "without options", strerror(errno));
                return false;
            }
        }
    }
#endif /* HAS_INSTALL_ENV */

    /* Handle fstab.  We're guaranteed to have a /target since mount has
     * already ran and /target is the first mount done.
     */
    output_info("installfile:" + std::to_string(this->lineno()),
                "mount: adding " + this->mountpoint() + " to /etc/fstab");
    char pass = (this->mountpoint() == "/" ? '1' : '0');
    const std::string fstab_opts = (this->options().empty() ?
                                        "defaults" : this->options());
    if(options.test(Simulate)) {
        if(this->mountpoint() == "/") {
            std::cout << "mkdir -p /target/etc" << std::endl;
        }
        std::cout << "printf '%s\\t%s\\t%s\\t%s\\t0\\t" << pass << "\\"
                  << "n' " << this->device() << " " << this->mountpoint()
                  << " " << fstype << " " << fstab_opts
                  << " >> /target/etc/fstab" << std::endl;
    }
#ifdef HAS_INSTALL_ENV
    else {
        if(this->mountpoint() == "/") {
            /* failure of mkdir will be handled in the !fstab_f case */
            mkdir("/target/etc",
                  S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH);
        }
        std::ofstream fstab_f("/target/etc/fstab");
        if(!fstab_f) {
            output_error("installfile:" + std::to_string(this->lineno()),
                         "mount: failure opening /etc/fstab for writing");
            return false;
        }
        fstab_f << this->device() << "\t" << this->mountpoint() << "\t"
                << fstype << "\t" << fstab_opts << "\t0\t" << pass
                << std::endl;
    }
#endif /* HAS_INSTALL_ENV */

    return true;
}
