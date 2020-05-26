/*
 * disk.hh - Definition of the Key classes for disk manipulation
 * libhscript, the HorizonScript library for
 * Project Horizon
 *
 * Copyright (c) 2019-2020 Adélie Linux and contributors.  All rights reserved.
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

    DiskId(const Script *_s, const ScriptLocation &_p,
           const std::string &my_block, const std::string &my_i) :
        Key(_s, _p), _block(my_block), _ident(my_i) {}
public:
    /*! Retrieve the block device that this key identifies. */
    const std::string device() const { return this->_block; }
    /*! Retrieve the identification for the block device. */
    const std::string ident() const { return this->_ident; }

    static Key *parseFromData(const std::string &, const ScriptLocation &,
                              int*, int*, const Script *);
    bool validate() const override;
    bool execute() const override;
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

    DiskLabel(const Script *_s, const ScriptLocation &_p, const std::string &_b,
              const LabelType &_t) :
        Key(_s, _p), _block(_b), _type(_t) {}
public:
    /*! Retrieve the block device that this key identifies. */
    const std::string device() const { return this->_block; }
    /*! Retrieve the type of disklabel for the block device. */
    LabelType type() const { return this->_type; }

    static Key *parseFromData(const std::string &, const ScriptLocation &,
                              int*, int*, const Script *);
    bool validate() const override;
    bool execute() const override;
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
        ESP,
        /*! GRUB BIOS (GPT only) */
        BIOS,
        /*! PReP boot (MBR only) */
        PReP
    };
private:
    const std::string _block;
    const int _partno;
    const SizeType _size_type;
    const uint64_t _size;
    const PartitionType _type;

    Partition(const Script *_sc, const ScriptLocation &_pos,
              const std::string &_b, const int _p, const SizeType _st,
              const uint64_t _s, const PartitionType _pt) :
        Key(_sc, _pos), _block(_b), _partno(_p), _size_type(_st), _size(_s),
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

    static Key *parseFromData(const std::string &, const ScriptLocation &,
                              int*, int*, const Script *);
    bool validate() const override;
    bool execute() const override;
};

class Encrypt : public Key {
private:
    const std::string _block;
    const std::string _pw;

    Encrypt(const Script *_s, const ScriptLocation &_pos, const std::string &_b,
            const std::string &_p) : Key(_s, _pos), _block(_b), _pw(_p) {}
public:
    /*! Retrieve the block device that this key encrypts. */
    const std::string device() const { return this->_block; }
    /*! Retrieve the passphrase used to encrypt the block device. */
    const std::string passphrase() const { return this->_pw; }

    static Key *parseFromData(const std::string &, const ScriptLocation &,
                              int*, int*, const Script *);
    bool validate() const override;
    bool execute() const override;
};

class LVMPhysical : public StringKey {
private:
    LVMPhysical(const Script *_s, const ScriptLocation &_p,
                const std::string &_d) : StringKey(_s, _p, _d) {}
public:
    static Key *parseFromData(const std::string &, const ScriptLocation &,
                              int*, int*, const Script *);
    bool execute() const override;
};

class LVMGroup : public Key {
private:
    const std::string _pv;
    const std::string _vgname;

    LVMGroup(const Script *_s, const ScriptLocation &_pos,
             const std::string &_p, const std::string &_v) :
        Key(_s, _pos), _pv(_p), _vgname(_v) {}
public:
    /*! Retrieve the physical volume where this volume group will reside. */
    const std::string pv() const { return this->_pv; }
    /*! Retrieve the name of this volume group. */
    const std::string name() const { return this->_vgname; }

    static Key *parseFromData(const std::string &, const ScriptLocation &,
                              int*, int*, const Script *);
    bool validate() const override;
    /*! Determine if the PV passed is a real one. */
    bool test_pv() const;
    bool execute() const override;
};

class LVMVolume : public Key {
private:
    const std::string _vg;
    const std::string _lvname;
    const SizeType _size_type;
    const uint64_t _size;

    LVMVolume(const Script *_sc, const ScriptLocation &_pos,
              const std::string &_v, const std::string &_n, SizeType _t,
              uint64_t _s) :
        Key(_sc, _pos), _vg(_v), _lvname(_n), _size_type(_t), _size(_s) {}
public:
    /*! Retrieve the volume group to which this volume belongs. */
    const std::string vg() const { return this->_vg; }
    /*! Retrieve the name of this volume. */
    const std::string name() const { return this->_lvname; }
    /*! Retrieve the type of size that this volume uses. */
    SizeType size_type() const { return this->_size_type; }
    /*! Retrieve the size of this volume. */
    uint64_t size() const { return this->_size; }

    static Key *parseFromData(const std::string &, const ScriptLocation &,
                              int*, int*, const Script *);
    bool validate() const override;
    bool execute() const override;
};

class Filesystem : public Key {
public:
    enum FilesystemType {
        Ext2,
        Ext3,
        Ext4,
        HFSPlus,
        JFS,
        VFAT,
        XFS
    };
private:
    const std::string _block;
    FilesystemType _type;

    Filesystem(const Script *_s, const ScriptLocation &_pos,
               const std::string &_b, FilesystemType _t) :
        Key(_s, _pos), _block(_b), _type(_t) {}
public:
    /*! Retrieve the block device on which to create the filesystem. */
    const std::string device() const { return this->_block; }
    /*! Retreive the type of filesystem to create. */
    FilesystemType fstype() const { return this->_type; }

    static Key *parseFromData(const std::string &, const ScriptLocation &,
                              int*, int*, const Script *);
    bool validate() const override;
    bool execute() const override;
};

class Mount : public Key {
private:
    const std::string _block;
    const std::string _mountpoint;
    const std::string _opts;

    Mount(const Script *_s, const ScriptLocation &_pos,
          const std::string &my_block, const std::string &my_mountpoint,
          const std::string &my_opts = "") :
        Key(_s, _pos), _block(my_block), _mountpoint(my_mountpoint),
        _opts(my_opts) {}
public:
    /*! Retrieve the block device to which this mount pertains. */
    const std::string device() const { return this->_block; }
    /*! Retrieve the mountpoint for this mount. */
    const std::string mountpoint() const { return this->_mountpoint; }
    /*! Retrieve the mount options for this mount, if any. */
    const std::string options() const { return this->_opts; }

    static Key *parseFromData(const std::string &, const ScriptLocation &,
                              int*, int*, const Script *);
    bool validate() const override;
    bool execute() const override;
};

}
}

#endif /* !__HSCRIPT_DISK_HH_ */
