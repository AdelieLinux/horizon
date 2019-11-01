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
    static Key *parseFromData(const std::string &, int, int *, int *);
    bool validate(ScriptOptions) const override;
    bool execute(ScriptOptions) const override;
};

class PkgInstall : public Key {
private:
    const std::set<std::string> _pkgs;
    PkgInstall(int _line, const std::set<std::string> my_pkgs) : Key(_line),
        _pkgs(my_pkgs) {}
public:
    static Key *parseFromData(const std::string &, int, int *, int *);
    const std::set<std::string> packages() const { return _pkgs; }
    bool validate(ScriptOptions) const override;
    bool execute(ScriptOptions) const override;
};

class Language : public StringKey {
private:
    Language(int _line, const std::string &my_lang) :
        StringKey(_line, my_lang) {}
public:
    static Key *parseFromData(const std::string &, int, int *, int *);
    bool execute(ScriptOptions) const override;
};

class Keymap : public StringKey {
private:
    Keymap(int _line, const std::string &keymap) :
        StringKey(_line, keymap) {}
public:
    static Key *parseFromData(const std::string &, int, int *, int *);
    bool validate(ScriptOptions) const override;
    bool execute(ScriptOptions) const override;
};

class Firmware : public BooleanKey {
private:
    Firmware(int _line, bool _value) : BooleanKey(_line, _value) {}
public:
    static Key *parseFromData(const std::string &, int, int *, int *);
    bool execute(ScriptOptions) const override;
};

class Timezone : public StringKey {
private:
    Timezone(int _line, const std::string &my_zone) :
        StringKey(_line, my_zone) {}
public:
    static Key *parseFromData(const std::string &, int, int *, int *);
    bool execute(ScriptOptions) const override;
};

class Repository : public StringKey {
private:
    Repository(int _line, const std::string &my_url) :
        StringKey(_line, my_url) {}
public:
    static Key *parseFromData(const std::string &, int, int *, int *);
    bool validate(ScriptOptions) const override;
    bool execute(ScriptOptions) const override;
};

class SigningKey : public StringKey {
private:
    SigningKey(int _line, const std::string &_path) :
        StringKey(_line, _path) {}
public:
    static Key *parseFromData(const std::string &, int, int *, int *);
    bool validate(ScriptOptions) const override;
    bool execute(ScriptOptions) const override;
};

}
}

#endif /* !__HSCRIPT_META_HH_ */
