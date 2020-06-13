/*
 * meta.hh - Definition of the Key classes for system metadata
 * libhscript, the HorizonScript library for
 * Project Horizon
 *
 * Copyright (c) 2019-2020 Adélie Linux and contributors.  All rights reserved.
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
    Hostname(const Script *_s, const ScriptLocation &_pos,
             const std::string my_name) : StringKey(_s, _pos, my_name) {}
public:
    static Key *parseFromData(const std::string &, const ScriptLocation &,
                              int *, int *, const Script *);
    bool validate() const override;
    bool execute() const override;
};

class Arch : public StringKey {
private:
    Arch(const Script *_s, const ScriptLocation &_p, const std::string &arch) :
        StringKey(_s, _p, arch) {}
public:
    static Key *parseFromData(const std::string &, const ScriptLocation &,
                              int *, int *, const Script *);
    bool execute() const override;
};

class PkgInstall : public Key {
private:
    const std::set<std::string> _pkgs;
    PkgInstall(const Script *_s, const ScriptLocation &_pos,
               const std::set<std::string> my_pkgs) : Key(_s, _pos),
        _pkgs(my_pkgs) {}
public:
    static Key *parseFromData(const std::string &, const ScriptLocation &,
                              int *, int *, const Script *);
    const std::set<std::string> packages() const { return _pkgs; }
    bool validate() const override;
    bool execute() const override;
};

class Language : public StringKey {
private:
    Language(const Script *_s, const ScriptLocation &_pos,
             const std::string &my_lang) : StringKey(_s, _pos, my_lang) {}
public:
    static Key *parseFromData(const std::string &, const ScriptLocation &,
                              int *, int *, const Script *);
    bool execute() const override;
};

class Keymap : public StringKey {
private:
    Keymap(const Script *_s, const ScriptLocation &_pos,
           const std::string &keymap) : StringKey(_s, _pos, keymap) {}
public:
    static Key *parseFromData(const std::string &, const ScriptLocation &,
                              int *, int *, const Script *);
    bool validate() const override;
    bool execute() const override;
};

class Firmware : public BooleanKey {
private:
    Firmware(const Script *_s, const ScriptLocation &_pos, bool _value) :
        BooleanKey(_s, _pos, _value) {}
public:
    static Key *parseFromData(const std::string &, const ScriptLocation &,
                              int *, int *, const Script *);
    bool execute() const override;
};

class Timezone : public StringKey {
private:
    Timezone(const Script *_s, const ScriptLocation &_pos,
             const std::string &my_zone) : StringKey(_s, _pos, my_zone) {}
public:
    static Key *parseFromData(const std::string &, const ScriptLocation &,
                              int *, int *, const Script *);
    bool execute() const override;
};

class Repository : public StringKey {
private:
    Repository(const Script *_s, const ScriptLocation &_pos,
               const std::string &my_url) : StringKey(_s, _pos, my_url) {}
public:
    static Key *parseFromData(const std::string &, const ScriptLocation &,
                              int *, int *, const Script *);
    bool validate() const override;
    bool execute() const override;
};

class SigningKey : public StringKey {
private:
    SigningKey(const Script *_s, const ScriptLocation &_pos,
               const std::string &_path) : StringKey(_s, _pos, _path) {}
public:
    static Key *parseFromData(const std::string &, const ScriptLocation &,
                              int *, int *, const Script *);
    bool validate() const override;
    bool execute() const override;
};

class SvcEnable : public StringKey {
private:
    SvcEnable(const Script *_s, const ScriptLocation &_pos,
              const std::string &_svc) : StringKey(_s, _pos, _svc) {}
public:
    static Key *parseFromData(const std::string &, const ScriptLocation &,
                              int *, int *, const Script *);
    bool execute() const override;
};

class Version : public StringKey {
private:
    Version(const Script *_s, const ScriptLocation &_p, const std::string &_v) :
        StringKey(_s, _p, _v) {}
public:
    static Key *parseFromData(const std::string &, const ScriptLocation &,
                              int *, int *, const Script *);
    bool execute() const override;
};

class Bootloader : public StringKey {
private:
    Bootloader(const Script *_s, const ScriptLocation &_p,
               const std::string &_v) : StringKey(_s, _p, _v) {}
public:
    static Key *parseFromData(const std::string &, const ScriptLocation &,
                              int *, int *, const Script *);
    bool validate() const override;
    bool execute() const override;
};

}
}

#endif /* !__HSCRIPT_META_HH_ */
