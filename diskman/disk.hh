/*
 * disk.hh - Public definition of the Disk class
 * diskman, the Disk Manipulation library for
 * Project Horizon
 *
 * Copyright (c) 2020 Adélie Linux and contributors.  All rights reserved.
 * This code is licensed under the AGPL 3.0 license, as noted in the
 * LICENSE-code file in the root directory of this repository.
 *
 * SPDX-License-Identifier: AGPL-3.0-only
 */

#ifndef DISKMAN__DISK_HH
#define DISKMAN__DISK_HH

#include <string>
#include <vector>

#include "partition.hh"

namespace Horizon {
namespace DiskMan {

/*! Represents a fixed disk device. */
class Disk {
public:
    /*! Potential disk label types */
    enum Label {
        /*! GUID Partition Table, common on x86 and POWER */
        GPT,
        /*! Master Boot Record, common on x86 and ARM */
        MBR,
        /*! Apple Partition Map, common on POWER */
        APM,
        /*! Unknown disk label type */
        Unknown
    };

    /*! Retrieve the disk's name.  For instance, "sda" or "nvme0n1". */
    const std::string name() const { return this->_name; }
    /*! Retrieve the disk's model.  This is specified by the vendor. */
    const std::string model() const { return this->_model; }
    /*! Retrieve the disk's serial number.  This may be equivalent to
     * the model name, if the vendor did not specify a serial number. */
    const std::string serial() const { return this->_full_serial; }
    /*! Retrieve the device node for this disk.  For instance, "/dev/sda". */
    const std::string node() const { return this->_node; }
    /*! Retrieve the system device path for this disk. */
    const std::string dev_path() const { return this->_devpath; }

    /*! Determine if this disk has a disk label attached. */
    bool has_label() const { return this->_has_label; }
    /*! Retrieve the type of disk label in use on this disk.
     *  Only valid if has_label() is true. */
    enum Label label() const { return this->_label; }

    /*! Determine if this disk has a file system written to it.
     *  If this method returns true, the file system is directly written to
     *  the disk; it is not a partition inside a disklabel. */
    bool has_fs() const { return this->_has_fs; }
    /*! Retrieve the type of file system written on this disk.
     *  Only valid if has_fs() is true. */
    const std::string fs_type() const { return this->_fs_type; }
    /*! Retrieve the label of the file system written on this disk.
     *  Only valid if has_fs() is true. */
    const std::string fs_label() const { return this->_fs_label; }

    /*! Retrieve the total size of the disk.
     * @returns The size of the disk, in mebibytes (MiB). */
    uint32_t total_size() const { return this->total_mb; }
    /*! Retrieve the amount of free space available on the disk.
     * @returns The size of free space on the disk, in mebibytes (MiB). */
    uint32_t free_space() const { return this->free_mb; }
    /*! Retrieve the size of the largest contiguous block of free space on
     *  the disk.
     * @returns The size of the largest contiguous free space block on the
     *          disk, in mebibytes (MiB). */
    uint32_t contiguous_block() const { return this->contiguous_mb; }

    /*! Retrieve the sector size of the disk. */
    uint32_t sector_size() const { return this->_sector; }

    /*! Retrieve the partitions contained in the label, if any.
     * @note You may only call this method if *has_label* is true. */
    const std::vector<Partition> partitions() const;

    /*! Attempt to (re)load the partition table from the disk.
     * @returns true if the partition table was loaded successfully.
     * @note The partition array will not be modified if a failure occurs. */
    bool reload_partitions();
private:
    /*! The name of the disk ("sda") */
    std::string _name;
    /*! The model name of the disk ("WDC WDBNCE2500PNC") */
    std::string _model;
    /*! The full serial number of the disk */
    std::string _full_serial;
    /*! The device node of the disk ("/dev/sda") */
    std::string _node;
    /*! The device path of the disk ("/sys/devices/...") */
    std::string _devpath;

    /*! Whether this disk has a disklabel */
    bool _has_label;
    /*! The type of disk label used, if +has_label+ is true */
    enum Label _label;
    /*! Partitions inside the disklabel */
    std::vector<Partition> _partitions;

    /*! Whether this disk has a direct filesystem (non-labelled) */
    bool _has_fs;
    /*! The type of file system in use, if +has_fs+ is true */
    std::string _fs_type;
    /*! The label of the file system, if +has_fs+ is true */
    std::string _fs_label;

    /*! Total size of this disk, in mebibytes */
    uint32_t total_mb;
    /*! Free space on this disk, in mebibytes */
    uint32_t free_mb;
    /*! Largest contiguous block of free space on this disk, in mebibytes */
    uint32_t contiguous_mb;
    /*! Size of this disk's sectors, in bytes */
    uint32_t _sector;

    Disk(void *creation, int type, bool partition);
    friend class DiskMan;
};

}
}

#endif  /* !DISKMAN__DISK_HH */
