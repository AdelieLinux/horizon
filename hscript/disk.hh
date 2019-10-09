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
};

class DiskLabel : public Key {
};

class Partition : public Key {
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

    Mount(int _line, std::string my_block, std::string my_mountpoint,
          std::string my_opts = "") : Key(_line), _block(my_block),
        _mountpoint(my_mountpoint), _opts(my_opts) {}
public:
    /*! Retrieve the block device to which this mount pertains. */
    const std::string device() const { return this->_block; }
    /*! Retrieve the mountpoint for this mount. */
    const std::string mountpoint() const { return this->_mountpoint; }
    /*! Retrieve the mount options for this mount, if any. */
    const std::string options() const { return this->_opts; }

    static Key *parseFromData(const std::string data, int lineno, int *errors,
                              int *warnings);
    bool validate() const override;
    bool execute() const override;
};

}
}

#endif /* !__HSCRIPT_DISK_HH_ */
