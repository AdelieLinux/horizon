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

#ifndef __HSCRIPT_USER_HH_
#define __HSCRIPT_USER_HH_

#include "key.hh"

namespace Horizon {
namespace Keys {

class RootPassphrase : public Key {
};

class Username : public Key {
};

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

#endif /* !__HSCRIPT_USER_HH_ */
