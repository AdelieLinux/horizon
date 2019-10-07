/*
 * disk.hh - Definition of the disk Key classes
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
};

}
}

#endif /* !__HSCRIPT_DISK_HH_ */
