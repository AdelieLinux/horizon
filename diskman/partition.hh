/*
 * partition.hh - Definition of the Partition class
 * diskman, the Disk Manipulation library for
 * Project Horizon
 *
 * Copyright (c) 2020 Adélie Linux and contributors.  All rights reserved.
 * This code is licensed under the AGPL 3.0 license, as noted in the
 * LICENSE-code file in the root directory of this repository.
 *
 * SPDX-License-Identifier: AGPL-3.0-only
 */

#ifndef DISKMAN__PARTITION_HH
#define DISKMAN__PARTITION_HH

#include <cstdint>
#include <string>

namespace Horizon {
namespace DiskMan {

class Disk;

class Partition {
public:
    /*! Retrieve the size of the partition in bytes. */
    uint64_t size() const { return this->_size; };
    /*! Retrieve the file system type of this partition. */
    const std::string fstype() const { return this->_fs_type; }
    /*! Retrieve the label of the file system on this partition. */
    const std::string label() const { return this->_label; }
    /*! Retrieve the device node of this partition (for example, /dev/sda1). */
    const std::string node() const { return this->_node; }
private:
    /*! The size of the partition, in bytes. */
    uint64_t _size;
    /*! The type of the file system on this partition (if any) */
    std::string _fs_type;
    /*! The label of the file system on this partition (if any) */
    std::string _label;
    /*! The device node of this partition. */
    std::string _node;
    Partition(Disk &d, void *creation, int type);
    friend class Disk;
};

}
}

#endif  /* !DISKMAN__PARTITION_HH */
