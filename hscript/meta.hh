/*
 * meta.hh - Definition of the Key classes for system metadata
 * libhscript, the HorizonScript library for
 * Project Horizon
 *
 * Copyright (c) 2019 Adélie Linux and contributors.  All rights reserved.
 * This code is licensed under the AGPL 3.0 license, as noted in the
 * LICENSE-code file in the root directory of this repository.
 *
 * SPDX-License-Identifier: AGPL-3.0-only
 */

#ifndef __HSCRIPT_META_HH_
#define __HSCRIPT_META_HH_

#include <string>
#include "key.hh"

namespace Horizon {
namespace Keys {

class Hostname : public Key {
private:
    const std::string _name;
    Hostname(int _line, const std::string my_name) : Key(_line),
        _name(my_name) {}
public:
    static Key *parseFromData(const std::string data, int lineno, int *errors,
                              int *warnings);
    const std::string name() const { return this->_name; }
    bool validate() const override;
    bool execute() const override;
};

class PkgInstall : public Key {
};

class Language : public Key {
};

class Keymap : public Key {
};

class Firmware : public Key {
};

class Timezone : public Key {
};

class Repository : public Key {
};

class SigningKey : public Key {
};

}
}

#endif /* !__HSCRIPT_META_HH_ */
