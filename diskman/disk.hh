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

#include <memory>
#include <string>

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

    /*! Whether this disk has a direct filesystem (non-labelled) */
    bool _has_fs;
    /*! The type of file system in use, if +has_fs+ is true */
    std::string _fs_type;
    /*! The label of the file system, if +has_fs+ is true */
    std::string _fs_label;

    Disk(void *creation, bool partition);
    friend class DiskMan;
};

}
}

#endif  /* !DISKMAN__DISK_HH */
