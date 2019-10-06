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

#include "key.hh"

namespace Horizon {
namespace Keys {

class DiskId : public Key {
};

class DiskLabel : public Key {
};

class Partition : public Key {
};

class Filesystem : public Key {
};

class Mount : public Key {
};

}
}
