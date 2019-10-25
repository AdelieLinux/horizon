/*
 * disk.hh - Definition of the Key classes for disk manipulation
 * libhscript, the HorizonScript library for
 * Project Horizon
 *
 * Copyright (c) 2019 Adélie Linux and contributors.  All rights reserved.
 * This code is licensed under the AGPL 3.0 license, as noted in the
 * LICENSE-code file in the root directory of this repository.
 *
 * SPDX-License-Identifier: AGPL-3.0-only
 */

#ifndef __HSCRIPT_DISK_HH_
#define __HSCRIPT_DISK_HH_

#include "key.hh"

namespace Horizon {
namespace Keys {

class DiskId : public Key {
private:
    const std::string _block;
    const std::string _ident;

    DiskId(int _line, const std::string &my_block, const std::string &my_i) :
        Key(_line), _block(my_block), _ident(my_i) {}
public:
    /*! Retrieve the block device that this key identifies. */
    const std::string device() const { return this->_block; }
    /*! Retrieve the identification for the block device. */
    const std::string ident() const { return this->_ident; }

    static Key *parseFromData(const std::string &, int, int*, int*);
    bool validate(ScriptOptions) const override;
    bool execute(ScriptOptions) const override;
};

class DiskLabel : public Key {
public:
    /*! The type of disklabel. */
    enum LabelType {
        /*! Apple Partition Map (APM) */
        APM,
        /*! Master Boot Record (MBR) */
        MBR,
        /*! GUID Partition Table (GPT) */
        GPT
    };
private:
    const std::string _block;
    const LabelType _type;

    DiskLabel(int _line, const std::string &_b, const LabelType &_t) :
        Key(_line), _block(_b), _type(_t) {}
public:
    /*! Retrieve the block device that this key identifies. */
    const std::string device() const { return this->_block; }
    /*! Retrieve the type of disklabel for the block device. */
    const LabelType type() const { return this->_type; }

    static Key *parseFromData(const std::string &, int, int*, int*);
    bool validate(ScriptOptions) const override;
    bool execute(ScriptOptions) const override;
};

/*! The type of size a disk key has */
enum SizeType {
    /*! Sized in bytes */
    Bytes,
    /*! Sized as a percentage of the whole disk */
    Percent,
    /*! Fill the rest of the disk */
    Fill
};

class Partition : public Key {
public:
    /*! Valid partition type codes */
    enum PartitionType {
        /*! None (default) */
        None = 0,
        /*! Bootable */
        Boot,
        /*! EFI System Partition (GPT only) */
        ESP
    };
private:
    const std::string _block;
    const int _partno;
    const SizeType _size_type;
    const uint64_t _size;
    const PartitionType _type;

    Partition(int _line, const std::string &_b, const int _p,
              const SizeType _st, const uint64_t _s, const PartitionType _pt) :
        Key(_line), _block(_b), _partno(_p), _size_type(_st), _size(_s),
        _type(_pt) {}
public:
    /*! Retrieve the block device that this key identifies. */
    const std::string device() const { return this->_block; }
    /*! Retrieve the partition number that this key identifies. */
    int partno() const { return this->_partno; }
    /*! Retrieve the type of size that this partition uses. */
    SizeType size_type() const { return this->_size_type; }
    /*! Retrieve the size of this partition. */
    uint64_t size() const { return this->_size; }
    /*! Retrieve the Type Code of this partition, if any. */
    PartitionType type() const { return this->_type; }

    static Key *parseFromData(const std::string &, int, int*, int*);
    bool validate(ScriptOptions) const override;
    bool execute(ScriptOptions) const override;
};

class Encrypt : public Key {
};

class LVMPhysical : public Key {
};

class LVMGroup : public Key {
};

class LVMVolume : public Key {
};

class Filesystem : public Key {
};

class Mount : public Key {
private:
    const std::string _block;
    const std::string _mountpoint;
    const std::string _opts;

    Mount(int _line, const std::string &my_block,
          const std::string &my_mountpoint, const std::string &my_opts = "") :
        Key(_line), _block(my_block), _mountpoint(my_mountpoint),
        _opts(my_opts) {}
public:
    /*! Retrieve the block device to which this mount pertains. */
    const std::string device() const { return this->_block; }
    /*! Retrieve the mountpoint for this mount. */
    const std::string mountpoint() const { return this->_mountpoint; }
    /*! Retrieve the mount options for this mount, if any. */
    const std::string options() const { return this->_opts; }

    static Key *parseFromData(const std::string &, int, int*, int*);
    bool validate(ScriptOptions) const override;
    bool execute(ScriptOptions) const override;
};

}
}

#endif /* !__HSCRIPT_DISK_HH_ */
