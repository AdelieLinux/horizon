/*
 * disk.cc - Implementation of the Key classes for disk manipulation
 * libhscript, the HorizonScript library for
 * Project Horizon
 *
 * Copyright (c) 2019-2020 Adélie Linux and contributors.  All rights reserved.
 * This code is licensed under the AGPL 3.0 license, as noted in the
 * LICENSE-code file in the root directory of this repository.
 *
 * SPDX-License-Identifier: AGPL-3.0-only
 */

#include <algorithm>
#include <assert.h>         /* assert */
#include <cstring>          /* strerror */
#include <fstream>
#include <set>
#include <string>
#ifdef HAS_INSTALL_ENV
#   include <array>
#   include <blkid/blkid.h>    /* blkid_get_tag_value */
#   include "util/filesystem.hh"
#   include <libudev.h>        /* udev_* */
#   include <parted/parted.h>  /* ped_* */
#   include <sys/mount.h>      /* mount */
#   include <sys/stat.h>       /* stat */
#   include <sys/types.h>      /* S_* */
#   include <unistd.h>         /* access */
#endif /* HAS_INSTALL_ENV */
#include "disk.hh"
#include "util.hh"
#include "util/output.hh"

using namespace Horizon::Keys;

#ifdef HAS_INSTALL_ENV
/*! Determine if _block is a valid block device.
 * @param key       The key associated with this test.
 * @param line      The line number where the key exists.
 * @param _block    The path to test.
 * @returns true if _block is valid, false otherwise.
 * @note Will output_error if an error occurs.
 */
bool is_block_device(const std::string &key, long line,
                     const std::string &_block) {
    struct stat blk_stat;
    const char *block_c = _block.c_str();
    if(access(block_c, F_OK) != 0 || stat(block_c, &blk_stat) != 0) {
        output_error("installfile:" + std::to_string(line),
                     key + ": error opening device " + _block,
                     strerror(errno));
        return false;
    }
    if(!S_ISBLK(blk_stat.st_mode)) {
        output_error("installfile:" + std::to_string(line),
                     key + ": " + _block + " is not a valid block device");
        return false;
    }
    return true;
}
#endif /* HAS_INSTALL_ENV */


Key *DiskId::parseFromData(const std::string &data, int lineno, int *errors,
                           int *, const Script *script) {
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
    return new DiskId(script, lineno, block, ident);
}

bool DiskId::validate() const {
#ifdef HAS_INSTALL_ENV
    /* We only validate if running in an Installation Environment. */
    if(script->options().test(InstallEnvironment)) {
        /* Unlike 'mount', 'diskid' *does* require that the block device exist
         * before installation begins.  This test is always valid. */
        return is_block_device("diskid", this->lineno(), _block);
    }
#endif /* HAS_INSTALL_ENV */

    return true;
}

bool DiskId::execute() const {
    bool match = false;

    output_info("installfile:" + std::to_string(line),
                "diskid: Checking " + _block + " for identification string " +
                _ident);

    if(!script->options().test(InstallEnvironment)) return true;

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
    } else {
        output_error("installfile:" + std::to_string(line),
                     "diskid: failed to retrieve disk identification",
                     "cannot read disk information");
    }

    if(!match) {
        output_error("installfile:" + std::to_string(line),
                     "diskid: device does not match expected identification "
                     "string");
    }

    udev_device_unref(device);
    udev_unref(udev);
#endif /* HAS_INSTALL_ENV */

    return match;  /* LCOV_EXCL_LINE */
}


Key *DiskLabel::parseFromData(const std::string &data, int lineno, int *errors,
                              int *, const Script *script) {
    std::string block, label;
    std::string::size_type sep = data.find_first_of(' ');
    LabelType type;

    /* REQ: Runner.Validate.disklabel.Validity */
    if(sep == std::string::npos || data.length() == sep + 1) {
        if(errors) *errors += 1;
        output_error("installfile:" + std::to_string(lineno),
                     "disklabel: expected a label type",
                     "valid format for disklabel is: [disk] [type]");
        return nullptr;
    }

    block = data.substr(0, sep);
    label = data.substr(sep + 1);
    std::transform(label.begin(), label.end(), label.begin(), ::tolower);
    /* REQ: Runner.Validate.disklabel.LabelType */
    if(label == "apm") {
        type = APM;
    } else if(label == "mbr") {
        type = MBR;
    } else if(label == "gpt") {
        type = GPT;
    } else {
        if(errors) *errors += 1;
        output_error("installfile:" + std::to_string(lineno),
                     "disklabel: '" + label + "' is not a valid label type",
                     "valid label types are: apm, mbr, gpt");
        return nullptr;
    }

    return new DiskLabel(script, lineno, block, type);
}

bool DiskLabel::validate() const {
#ifdef HAS_INSTALL_ENV
    /* REQ: Runner.Validate.disklabel.Block */
    if(script->options().test(InstallEnvironment)) {
        /* disklabels are created before any others, so we can check now */
        return is_block_device("disklabel", this->lineno(), _block);
    }
#endif /* HAS_INSTALL_ENV */

    return true;
}

bool DiskLabel::execute() const {
    std::string type_str;
    switch(this->type()) {
    case APM:
        type_str = "apm";
        break;
    case MBR:
        type_str = "msdos";
        break;
    case GPT:
        type_str = "gpt";
        break;
    }

    output_info("installfile:" + std::to_string(this->lineno()),
                "disklabel: creating new " + type_str + " disklabel on " +
                device());

    if(script->options().test(Simulate)) {
        std::cout << "parted -ms " << this->device() << " mklabel "
                  << type_str << std::endl;
        return true;
    }

#ifdef HAS_INSTALL_ENV
    PedDevice *pdevice = ped_device_get(this->device().c_str());
    PedDiskType *label = ped_disk_type_get(type_str.c_str());
    int res;

    if(label == nullptr) {
        output_error("installfile:" + std::to_string(this->lineno()),
                     "disklabel: Parted does not support label type " +
                     type_str + "!");
        return false;
    }

    /* REQ: Runner.Execute.disklabel.Overwrite */
    ped_disk_clobber(pdevice);
    PedDisk *disk = ped_disk_new_fresh(pdevice, label);
    if(disk == nullptr) {
        output_error("installfile:" + std::to_string(this->lineno()),
                     "disklabel: internal error creating new " +
                     type_str + " disklabel on " + _block);
        return false;
    }

    res = ped_disk_commit(disk);
    if(res != 1) {
        output_error("installfile:" + std::to_string(this->lineno()),
                     "disklabel: error creating disklabel on " + _block);
    }
    return (res == 1);
#else
    return false;  /* LCOV_EXCL_LINE */
#endif /* HAS_INSTALL_ENV */
}


Key *Encrypt::parseFromData(const std::string &data, int lineno, int *errors,
                            int *, const Script *script) {
    std::string::size_type sep = data.find(' ');
    std::string dev, pass;

    if(sep == std::string::npos) {
        dev = data;
    } else {
        dev = data.substr(0, sep);
        pass = data.substr(sep + 1);
    }

    if(dev.size() < 6 || dev.compare(0, 5, "/dev/")) {
        if(errors) *errors += 1;
        output_error("installfile:" + std::to_string(lineno),
                     "encrypt: expected path to block device");
        return nullptr;
    }

    return new Encrypt(script, lineno, dev, pass);
}

bool Encrypt::validate() const {
    return true;
}

bool Encrypt::execute() const {
    return false;
}


/*! Parse a size string into a size and type.
 * @param   in_size     (in) The string to parse.
 * @param   out_size    (out) Where to which to write the size in bytes or %.
 * @param   type        (out) The type of size determined.
 * @returns true if the string was parseable, false otherwise.
 */
bool parse_size_string(const std::string &in_size, uint64_t *out_size,
                       SizeType *type) {
    std::string size(in_size), numbers, suffix;
    std::string::size_type suffix_pos;
    uint64_t multiplicand = 0;

    /* Validate parameters */
    assert(out_size != nullptr);
    assert(type != nullptr);

    /* Simpler since the string isn't case-sensitive. */
    std::transform(size.cbegin(), size.cend(), size.begin(), ::tolower);

    if(size == "fill") {
        /* That was easy™ */
        *type = SizeType::Fill;
        *out_size = 0;
        return true;
    }

    if(size.size() <= 1) {
        /* at least two characters are required:
         * - a 9 byte partition is invalid
         */
        return false;
    }

    if(size.size() > 21) {
        output_error("partition", "Value too large");
        return false;
    }

    suffix_pos = size.find_first_not_of("12345667890");
    /* this is always correct unless suffix is %, which is handled below */
    *type = SizeType::Bytes;
    try {
        *out_size = std::stoul(size.substr(0, suffix_pos));
    } catch(const std::exception &) {
        /* something is wrong; throw the same error as a non-numeric value */
        suffix_pos = 0;
    }

    if(suffix_pos == std::string::npos) {
        output_warning("partition", "size has no suffix; assuming bytes");
        return true;
    }

    if(suffix_pos == 0) {
        output_error("partition", "size must be a whole number, "
                     "followed by optional suffix [K|M|G|T|%]");
        return false;
    }

    suffix = size.substr(suffix_pos);

#define OVERFLOW_ON(MAX_VAL) \
    if(*out_size > MAX_VAL) {\
        output_error("partition", "Value too large");\
        return false;\
    }

    switch(suffix[0]) {
    case 'k':
        multiplicand = 1024;
        OVERFLOW_ON(0x3FFFFFFFFFFFFF)
        break;
    case 'm':
        multiplicand = 1048576;
        OVERFLOW_ON(0xFFFFFFFFFFF)
        break;
    case 'g':
        multiplicand = 1073741824;
        OVERFLOW_ON(0x3FFFFFFFF)
        break;
    case 't':
        multiplicand = 1099511627776;
        OVERFLOW_ON(0xFFFFFF)
        break;
    case '%':
        *type = SizeType::Percent;
        multiplicand = 1;
        OVERFLOW_ON(100)
        break;
    }

#undef OVERFLOW_ON

    /* if multiplicand is 0, it's an invalid suffix. */
    if(suffix.size() != 1 || multiplicand == 0) {
        output_error("partition", "size suffix must be K, M, G, T, or %");
        return false;
    }

    *out_size *= multiplicand;
    return true;
}


Key *Partition::parseFromData(const std::string &data, int lineno, int *errors,
                              int *, const Script *script) {
    std::string block, pno, size_str, typecode;
    std::string::size_type next_pos, last_pos;
    int part_no;
    SizeType size_type;
    uint64_t size;
    PartitionType type = None;

    long spaces = std::count(data.cbegin(), data.cend(), ' ');
    if(spaces < 2 || spaces > 3) {
        if(errors) *errors += 1;
        output_error("installfile:" + std::to_string(lineno),
                     "partition: expected either 3 or 4 elements, got: " +
                     std::to_string(spaces),
                     "syntax is: partition [block] [#] [size] ([type])");
        return nullptr;
    }

    last_pos = next_pos = data.find_first_of(' ');
    block = data.substr(0, next_pos);

    if(block.compare(0, 4, "/dev")) {
        if(errors) *errors += 1;
        output_error("installfile:" + std::to_string(lineno),
                     "partition: expected path to block device",
                     "'" + block + "' is not a valid block device path");
        return nullptr;
    }

    next_pos = data.find_first_of(' ', last_pos + 1);
    pno = data.substr(last_pos + 1, next_pos - last_pos);
    try {
        part_no = std::stoi(pno);
    } catch(const std::exception &) {
        if(errors) *errors += 1;
        output_error("installfile:" + std::to_string(lineno),
                     "partition: expected partition number, got", pno);
        return nullptr;
    }
    last_pos = next_pos;
    next_pos = data.find_first_of(' ', last_pos + 1);
    if(next_pos == std::string::npos) {
        size_str = data.substr(last_pos + 1);
    } else {
        size_str = data.substr(last_pos + 1, next_pos - last_pos - 1);
        typecode = data.substr(next_pos + 1);
    }
    if(!parse_size_string(size_str, &size, &size_type)) {
        if(errors) *errors += 1;
        output_error("installfile:" + std::to_string(lineno),
                     "partition: invalid size", size_str);
        return nullptr;
    }

    if(!typecode.empty()) {
        std::transform(typecode.cbegin(), typecode.cend(), typecode.begin(),
                       ::tolower);
        if(typecode == "boot") {
            type = Boot;
        } else if(typecode == "esp") {
            type = ESP;
        } else if(typecode == "bios") {
            type = BIOS;
        } else if(typecode == "prep") {
            type = PReP;
        } else {
            if(errors) *errors += 1;
            output_error("installfile:" + std::to_string(lineno),
                         "partition: expected type code, got: " + typecode,
                         "valid type codes are: boot esp bios prep");
            return nullptr;
        }
    }

    return new Partition(script, lineno, block, part_no, size_type, size, type);
}

bool Partition::validate() const {
#ifdef HAS_INSTALL_ENV
    if(script->options().test(InstallEnvironment)) {
        /* REQ: Runner.Validate.partition.Block */
        return is_block_device("partition", this->lineno(), this->device());
    }
#endif /* HAS_INSTALL_ENV */
    return true;
}

bool Partition::execute() const {
    output_info("installfile:" + std::to_string(this->lineno()),
                "partition: creating partition #" + std::to_string(_partno) +
                " on " + _block);

    if(script->options().test(Simulate)) {
        output_error("installfile:" + std::to_string(this->lineno()),
                     "partition: Not supported in Simulation mode");
        return true;
    }

#ifdef HAS_INSTALL_ENV
    PedDevice *dev = ped_device_get(this->device().c_str());
    if(dev == nullptr) {
        output_error("installfile:" + std::to_string(this->lineno()),
                     "partition: error opening device " + this->device());
        return false;
    }

    PedDisk *disk = ped_disk_new(dev);
    if(disk == nullptr) {
        output_error("installfile:" + std::to_string(this->lineno()),
                     "partition: error reading device " + this->device());
        return false;
    }

    int last = ped_disk_get_last_partition_num(disk);

    /* no partitions = 0 partitions */
    if(last == -1) last = 0;

    if(last != (this->partno() - 1)) {
        output_error("installfile:" + std::to_string(this->lineno()),
                     "partition: consistency error on " + this->device(),
                     "Partition #" + std::to_string(this->partno()) +
                     " has been requested, but the disk has " +
                     std::to_string(last) + " partitions");
        ped_disk_destroy(disk);
        return false;
    }

    PedPartition *before, *me;
    PedSector start = 0;
    PedSector size = 0;
    if(last > 0) {
        before = ped_disk_get_partition(disk, last);
        if(before == nullptr) {
            output_error("installfile:" + std::to_string(this->lineno()),
                         "partition: error reading partition table on " +
                         this->device());
            ped_disk_destroy(disk);
            return false;
        }
        start = before->geom.end + 1;
    }
    /* Ensure the first MiB is free for various firmware and boot software
     * that use it. */
    if(start < 2048) start = 2048;

    switch(this->size_type()) {
    case SizeType::Bytes:
        size = static_cast<int64_t>(this->size()) / dev->sector_size;
        break;
    case SizeType::Percent:
        size = dev->length * (this->size() / 100.0);
        break;
    case SizeType::Fill:
        size = dev->length - start;
        break;
    }

    me = ped_partition_new(disk, PED_PARTITION_NORMAL, nullptr,
                           start, start + size);
    if(me == nullptr) {
        output_error("installfile:" + std::to_string(this->lineno()),
                     "partition: error creating partition on " +
                     this->device());
        ped_disk_destroy(disk);
        return false;
    }

    switch(_type) {
    case Boot:
        ped_partition_set_flag(me, PED_PARTITION_BOOT, 1);
        break;
    case ESP:
        ped_partition_set_flag(me, PED_PARTITION_ESP, 1);
        break;
    case BIOS:
        ped_partition_set_flag(me, PED_PARTITION_BIOS_GRUB, 1);
        break;
    case PReP:
        ped_partition_set_flag(me, PED_PARTITION_PREP, 1);
        break;
    case None:
        /* we good */
        break;
    }

    int res = ped_disk_add_partition(disk, me, ped_constraint_any(dev));
    if(res == 0) {
        output_error("installfile:" + std::to_string(this->lineno()),
                     "partition: error adding partition to " +
                     this->device());
        ped_disk_destroy(disk);
        return false;
    }

    res = ped_disk_commit(disk);
    if(res != 1) {
        output_error("installfile:" + std::to_string(this->lineno()),
                     "partition: error flushing changes to " +
                     this->device());
        ped_disk_destroy(disk);
        return false;
    }

    ped_disk_destroy(disk);
#endif /* HAS_INSTALL_ENV */
    return true;  /* LCOV_EXCL_LINE */
}


const static std::set<std::string> valid_fses = {
    "ext2", "ext3", "ext4", "hfs+", "jfs", "vfat", "xfs"
};


Key *Filesystem::parseFromData(const std::string &data, int lineno,
                               int *errors, int *, const Script *script) {
    if(std::count(data.begin(), data.end(), ' ') != 1) {
        if(errors) *errors += 1;
        output_error("installfile:" + std::to_string(lineno),
                     "fs: expected exactly two elements",
                     "syntax is: fs [device] [fstype]");
        return nullptr;
    }

    std::string::size_type sep = data.find(' ');
    std::string device(data.substr(0, sep));
    std::string fstype(data.substr(sep + 1));
    FilesystemType type;

    if(device.size() < 6 || device.compare(0, 5, "/dev/")) {
        if(errors) *errors += 1;
        output_error("installfile:" + std::to_string(lineno),
                     "fs: element 1: expected device node",
                     "'" + device + "' is not a valid device node");
        return nullptr;
    }

    if(valid_fses.find(fstype) == valid_fses.end()) {
        std::string fses;
        for(auto &&fs : valid_fses) fses += fs + " ";

        if(errors) *errors += 1;
        output_error("installfile:" + std::to_string(lineno),
                     "fs: element 2: expected filesystem type",
                     "valid filesystems are: " + fses);
        return nullptr;
    }

    if(fstype == "ext2") {
        type = Ext2;
    } else if(fstype == "ext3") {
        type = Ext3;
    } else if(fstype == "ext4") {
        type = Ext4;
    } else if(fstype == "hfs+") {
        type = HFSPlus;
    } else if(fstype == "jfs") {
        type = JFS;
    } else if(fstype == "vfat") {
        type = VFAT;
    } else {
        type = XFS;
    }

    return new Filesystem(script, lineno, device, type);
}

bool Filesystem::validate() const {
    /* Validation is done during parsing. */
    return true;
}

bool Filesystem::execute() const {
    std::string cmd;
    std::vector<std::string> args;

    output_info("installfile:" + std::to_string(line),
                "fs: creating new filesystem on " + _block);

    switch(_type) {
    case Ext2:
        cmd = "mkfs.ext2";
        break;
    case Ext3:
        cmd = "mkfs.ext3";
        break;
    case Ext4:
        cmd = "mkfs.ext4";
        break;
    case HFSPlus:
        cmd = "mkfs.hfsplus";
        args.push_back("-w");
        break;
    case JFS:
        cmd = "mkfs.jfs";
        args.push_back("-q");
        break;
    case VFAT:
        cmd = "mkfs.vfat";
        args.push_back("-F32");
        break;
    case XFS:
        cmd = "mkfs.xfs";
        args.push_back("-f");
        break;
    }

    if(_type == Ext2 || _type == Ext3 || _type == Ext4) {
        const std::string dev_node(
                    device().substr(device().find_last_of('/') + 1));
        args.push_back("-q");
    }

    args.push_back(_block);

    if(script->options().test(Simulate)) {
        std::cout << cmd;
        for(auto &&arg : args) {
            std::cout << " " << arg;
        }
        std::cout << std::endl;
        return true;
    }

#ifdef HAS_INSTALL_ENV
    if(run_command(cmd, args) != 0) {
        output_error("installfile:" + std::to_string(line),
                     "fs: failed to create filesystem");
        return false;
    }
#endif /* HAS_INSTALL_ENV */
    return true;  /* LCOV_EXCL_LINE */
}


Key *Mount::parseFromData(const std::string &data, int lineno, int *errors,
                          int *, const Script *script) {
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

    return new Mount(script, lineno, dev, where, opt);
}

bool Mount::validate() const {
    return true;
}

bool Mount::execute() const {
    const std::string actual_mount(script->targetDirectory() +
                                   this->mountpoint());
    const char *fstype = nullptr;
#ifdef HAS_INSTALL_ENV
    error_code ec;
#endif

    /* We have to get the filesystem for the node. */
    if(script->options().test(Simulate)) {
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
    if(script->options().test(Simulate)) {
        std::cout << "mount ";
        if(!this->options().empty()) {
            std::cout << "-o " << this->options() << " ";
        }
        std::cout << this->device() << " " << actual_mount << std::endl;
    }
#ifdef HAS_INSTALL_ENV
    else {
        /* mount */
        if(!fs::exists(actual_mount, ec)) {
            fs::create_directory(actual_mount, ec);
            if(ec) {
                output_error("installfile:" + std::to_string(this->lineno()),
                             "mount: failed to create target directory for "
                             + this->mountpoint(), ec.message());
                return false;
            }
        }
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
    if(script->options().test(Simulate)) {
        if(this->mountpoint() == "/") {
            std::cout << "mkdir -p " << script->targetDirectory() << "/etc"
                      << std::endl;
        }
        std::cout << "printf '%s\\t%s\\t%s\\t%s\\t0\\t" << pass << "\\"
                  << "n' " << this->device() << " " << this->mountpoint()
                  << " " << fstype << " " << fstab_opts
                  << " >> " << script->targetDirectory() << "/etc/fstab"
                  << std::endl;
    }
#ifdef HAS_INSTALL_ENV
    else {
        if(this->mountpoint() == "/") {
            fs::create_directory(script->targetDirectory() + "/etc", ec);
            if(ec) {
                output_error("installfile:" + std::to_string(this->lineno()),
                             "mount: failed to create /etc for target",
                             ec.message());
                return false;
            }
            fs::permissions(script->targetDirectory() + "/etc", rwxr_xr_x,
                #if defined(FS_IS_STDCXX)
                            fs::perm_options::replace,
                #endif
                            ec);
            if(ec) {
                output_warning("installfile:" + std::to_string(this->lineno()),
                               "mount: failed to set permissions for target /etc",
                               ec.message());
            }
        }
        std::ofstream fstab_f(script->targetDirectory() + "/etc/fstab",
                              std::ios::app);
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
