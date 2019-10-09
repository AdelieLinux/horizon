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

class RootPassphrase : public StringKey {
private:
    RootPassphrase(int _line, const std::string my_pw) :
        StringKey(_line, my_pw) {}
public:
    static Key *parseFromData(const std::string data, int lineno, int *errors,
                              int *warnings);
    bool validate() const override;
    bool execute() const override;
};

class Username : public StringKey {
};

class UserAlias : public Key {
};

class UserPassphrase : public StringKey {
};

class UserIcon : public Key {
};

class UserGroups : public Key {
};

}
}

#endif /* !__HSCRIPT_USER_HH_ */
