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
#include <set>
#include "key.hh"

namespace Horizon {
namespace Keys {

class Hostname : public StringKey {
private:
    Hostname(int _line, const std::string my_name) :
        StringKey(_line, my_name) {}
public:
    static Key *parseFromData(const std::string data, int lineno, int *errors,
                              int *warnings);
    bool validate(ScriptOptions) const override;
    bool execute(ScriptOptions) const override;
};

class PkgInstall : public Key {
private:
    const std::set<std::string> _pkgs;
    PkgInstall(int _line, const std::set<std::string> my_pkgs) : Key(_line),
        _pkgs(my_pkgs) {}
public:
    static Key *parseFromData(const std::string data, int lineno, int *errors,
                              int *warnings);
    const std::set<std::string> packages() const { return _pkgs; }
    bool validate(ScriptOptions) const override { return true; }
    bool execute(ScriptOptions) const override { return true; }

};

class Language : public StringKey {
};

class Keymap : public StringKey {
};

class Firmware : public BooleanKey {
};

class Timezone : public StringKey {
};

class Repository : public Key {
};

class SigningKey : public Key {
};

}
}

#endif /* !__HSCRIPT_META_HH_ */
