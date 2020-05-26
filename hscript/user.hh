/*
 * user.hh - Definition of the Key classes for user account data
 * libhscript, the HorizonScript library for
 * Project Horizon
 *
 * Copyright (c) 2019-2020 Adélie Linux and contributors.  All rights reserved.
 * This code is licensed under the AGPL 3.0 license, as noted in the
 * LICENSE-code file in the root directory of this repository.
 *
 * SPDX-License-Identifier: AGPL-3.0-only
 */

#ifndef __HSCRIPT_USER_HH_
#define __HSCRIPT_USER_HH_

#include <set>
#include <string>
#include "key.hh"

namespace Horizon {
namespace Keys {

class RootPassphrase : public StringKey {
private:
    RootPassphrase(const Script *_s, const ScriptLocation &_p,
                   const std::string &my_pw) : StringKey(_s, _p, my_pw) {}
public:
    static Key *parseFromData(const std::string &, const ScriptLocation &,
                              int*, int*, const Script *);
    bool validate() const override;
    bool execute() const override;
};

class Username : public StringKey {
private:
    Username(const Script *_s, const ScriptLocation &_p,
             const std::string &name) : StringKey(_s, _p, name) {}
public:
    static Key *parseFromData(const std::string &, const ScriptLocation &,
                              int*, int*, const Script *);
    bool execute() const override;
};

class UserAlias : public Key {
private:
    const std::string _username;
    const std::string _alias;

    UserAlias(const Script *_s, const ScriptLocation &_p, const std::string &_n,
              const std::string &_a) : Key(_s, _p), _username(_n), _alias(_a) {}
public:
    static Key *parseFromData(const std::string &, const ScriptLocation &,
                              int*, int*, const Script *);

    /*! Retrieve the username for this alias. */
    const std::string &username() const { return this->_username; }
    /*! Retrieve the alias for the account. */
    const std::string &alias() const { return this->_alias; }

    bool validate() const override;
    bool execute() const override;
};

class UserPassphrase : public Key {
private:
    const std::string _username;
    const std::string _passphrase;

    UserPassphrase(const Script *_s, const ScriptLocation &_pos,
                   const std::string &_n, const std::string &_p) :
        Key(_s, _pos), _username(_n), _passphrase(_p) {}
public:
    static Key *parseFromData(const std::string &, const ScriptLocation &,
                              int*, int*, const Script *);

    /*! Retrieve the username for this passphrase. */
    const std::string &username() const { return this->_username; }
    /*! Retrieve the passphrase for the account. */
    const std::string &passphrase() const { return this->_passphrase; }

    bool validate() const override;
    bool execute() const override;
};

class UserIcon : public Key {
private:
    const std::string _username;
    const std::string _icon_path;

    UserIcon(const Script *_s, const ScriptLocation &_p, const std::string &_n,
             const std::string &_i) :
        Key(_s, _p), _username(_n), _icon_path(_i) {}
public:
    static Key *parseFromData(const std::string &, const ScriptLocation &,
                              int*, int*, const Script *);

    /*! Retrieve the username for this icon. */
    const std::string &username() const { return this->_username; }
    /*! Retrieve the icon path for the account. */
    const std::string &icon() const { return this->_icon_path; }

    bool validate() const override;
    bool execute() const override;
};

class UserGroups : public Key {
private:
    const std::string _username;
    const std::set<std::string> _groups;

    UserGroups(const Script *_s, const ScriptLocation &_pos,
               const std::string &_n, const std::set<std::string> &_g) :
        Key(_s, _pos), _username(_n), _groups(_g) {}
public:
    static Key *parseFromData(const std::string &, const ScriptLocation &,
                              int*, int*, const Script *);

    /*! Retrieve the username for this group set. */
    const std::string &username() const { return this->_username; }
    /*! Retrieve the groups for the account. */
    const std::set<std::string> &groups() const { return this->_groups; }

    bool validate() const override;
    bool execute() const override;
};

}
}

#endif /* !__HSCRIPT_USER_HH_ */
