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

#include <set>
#include <string>
#include "key.hh"

namespace Horizon {
namespace Keys {

class RootPassphrase : public StringKey {
private:
    RootPassphrase(int _line, const std::string &my_pw) :
        StringKey(_line, my_pw) {}
public:
    static Key *parseFromData(const std::string &, int, int*, int*);
    bool validate(ScriptOptions) const override;
    bool execute(ScriptOptions) const override;
};

class Username : public StringKey {
private:
    Username(int _line, const std::string &name) :
        StringKey(_line, name) {}
public:
    static Key *parseFromData(const std::string &, int, int*, int*);
    bool validate(ScriptOptions) const override;
    bool execute(ScriptOptions) const override;
};

class UserAlias : public Key {
private:
    const std::string _username;
    const std::string _alias;

    UserAlias(int _line, const std::string &_n, const std::string &_a) :
        Key(_line), _username(_n), _alias(_a) {}
public:
    static Key *parseFromData(const std::string &, int, int*, int*);

    /*! Retrieve the username for this alias. */
    const std::string &username() const { return this->_username; }
    /*! Retrieve the alias for the account. */
    const std::string &alias() const { return this->_alias; }

    bool validate(ScriptOptions) const override;
    bool execute(ScriptOptions) const override;
};

class UserPassphrase : public Key {
private:
    const std::string _username;
    const std::string _passphrase;

    UserPassphrase(int _line, const std::string &_n, const std::string &_p) :
        Key(_line), _username(_n), _passphrase(_p) {}
public:
    static Key *parseFromData(const std::string &, int, int*, int*);

    /*! Retrieve the username for this passphrase. */
    const std::string &username() const { return this->_username; }
    /*! Retrieve the passphrase for the account. */
    const std::string &passphrase() const { return this->_passphrase; }

    bool validate(ScriptOptions) const override;
    bool execute(ScriptOptions) const override;
};

class UserIcon : public Key {
private:
    const std::string _username;
    const std::string _icon_path;

    UserIcon(int _line, const std::string &_n, const std::string &_i) :
        Key(_line), _username(_n), _icon_path(_i) {}
public:
    static Key *parseFromData(const std::string &, int, int*, int*);

    /*! Retrieve the username for this icon. */
    const std::string &username() const { return this->_username; }
    /*! Retrieve the icon path for the account. */
    const std::string &icon() const { return this->_icon_path; }

    bool validate(ScriptOptions) const override;
    bool execute(ScriptOptions) const override;
};

class UserGroups : public Key {
private:
    const std::string _username;
    const std::set<std::string> _groups;

    UserGroups(int _line, const std::string &_n,
               const std::set<std::string> &_g) :
        Key(_line), _username(_n), _groups(_g) {}
public:
    static Key *parseFromData(const std::string &, int, int*, int*);

    /*! Retrieve the username for this group set. */
    const std::string &username() const { return this->_username; }
    /*! Retrieve the groups for the account. */
    const std::set<std::string> &groups() const { return this->_groups; }

    bool validate(ScriptOptions) const override;
    bool execute(ScriptOptions) const override;
};

}
}

#endif /* !__HSCRIPT_USER_HH_ */
