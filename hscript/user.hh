/*
 * user.hh - Definition of the Key classes for user account data
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

class UserAlias : public Key {
};

class UserPassphrase : public Key {
};

class UserIcon : public Key {
};

class UserGroups : public Key {
};

}
}
