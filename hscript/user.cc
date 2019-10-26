/*
 * user.cc - Implementation of the Key classes for user account data
 * libhscript, the HorizonScript library for
 * Project Horizon
 *
 * Copyright (c) 2019 Adélie Linux and contributors.  All rights reserved.
 * This code is licensed under the AGPL 3.0 license, as noted in the
 * LICENSE-code file in the root directory of this repository.
 *
 * SPDX-License-Identifier: AGPL-3.0-only
 */

#include <assert.h>
#include <cstring>
#include <fstream>
#include <set>
#include <sstream>
#include <time.h>
#include "user.hh"
#include "util/net.hh"
#include "util/output.hh"

using namespace Horizon::Keys;

const static std::set<std::string> system_names = {
    "root", "bin", "daemon", "adm", "lp", "sync", "shutdown", "halt", "mail",
    "news", "uucp", "operator", "man", "postmaster", "cron", "ftp", "sshd",
    "at", "squid", "xfs", "games", "postgres", "cyrus", "vpopmail", "utmp",
    "catchlog", "alias", "qmaild", "qmailp", "qmailq", "qmailr", "qmails",
    "qmaill", "ntp", "smmsp", "guest", "nobody"
};

const static std::set<std::string> system_groups = {
    "root", "bin", "daemon", "sys", "adm", "tty", "disk", "lp", "mem", "kmem",
    "wheel", "floppy", "mail", "news", "uucp", "man", "cron", "console",
    "audio", "cdrom", "dialout", "ftp", "sshd", "input", "at", "tape", "video",
    "netdev", "readproc", "squid", "xfs", "kvm", "games", "shadow", "postgres",
    "cdrw", "usb", "vpopmail", "users", "catchlog", "ntp", "nofiles", "qmail",
    "qmaill", "smmsp", "locate", "abuild", "utmp", "ping", "nogroup", "nobody"
};


/*
 * is_valid_name is from shadow libmisc/chkname.c:
 *
 * Copyright (c) 1990 - 1994, Julianne Frances Haugh
 * Copyright (c) 1996 - 2000, Marek Michałkiewicz
 * Copyright (c) 2001 - 2005, Tomasz Kłoczko
 * Copyright (c) 2005 - 2008, Nicolas François
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The name of the copyright holders or contributors may not be used to
 *    endorse or promote products derived from this software without
 *    specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 * PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT
 * HOLDERS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

static bool is_valid_name (const char *name)
{
        /*
         * User/group names must match [a-z_][a-z0-9_-]*[$]
         */
        if (('\0' == *name) ||
            !((('a' <= *name) && ('z' >= *name)) || ('_' == *name))) {
                return false;
        }

        while ('\0' != *++name) {
                if (!(( ('a' <= *name) && ('z' >= *name) ) ||
                      ( ('0' <= *name) && ('9' >= *name) ) ||
                      ('_' == *name) ||
                      ('-' == *name) ||
                      ('.' == *name) ||
                      ( ('$' == *name) && ('\0' == *(name + 1)) )
                     )) {
                        return false;
                }
        }

        return true;
}

/* End above copyright ^ */

/*! Determine if a string is a valid crypt passphrase
 * @param pw        The string to test for validity.
 * @param key       The name of key being validated ('rootpw', 'userpw', ...)
 * @param lineno    The line number where the key occurs.
 * @returns true if +pw+ is a valid crypt passphrase; false otherwise.
 */
static bool string_is_crypt(const std::string &pw, const std::string &key,
                            int lineno) {
    if(pw.size() < 5 || pw[0] != '$' || (pw[1] != '2' && pw[1] != '6')
            || pw[2] != '$') {
        output_error("installfile:" + std::to_string(lineno),
                     key + ": value is not a crypt-style encrypted passphrase");
        return false;
    }
    return true;
}


Key *RootPassphrase::parseFromData(const std::string &data, int lineno,
                                   int *errors, int *warnings) {
    if(!string_is_crypt(data, "rootpw", lineno)) {
        if(errors) *errors += 1;
        return nullptr;
    }
    return new RootPassphrase(lineno, data);
}

bool RootPassphrase::validate(ScriptOptions) const {
    /* XXX TODO: not sure what other validation we can / should do here. */
    return true;
}

bool RootPassphrase::execute(ScriptOptions options) const {
    const std::string root_line = "root:" + this->_value + ":" +
            std::to_string(time(nullptr) / 86400) + ":0:::::";

    output_info("installfile:" + std::to_string(this->lineno()),
                "rootpw: setting root passphrase");

    if(options.test(Simulate)) {
        std::cout << "(printf '" << root_line << "\\" << "n'; "
                  << "cat /target/etc/shadow | sed '1d') > /tmp/shadow"
                  << std::endl
                  << "mv /tmp/shadow /target/etc/shadow" << std::endl
                  << "chown root:shadow /target/etc/shadow" << std::endl
                  << "chmod 640 /target/etc/shadow" << std::endl;
        return true;
    }

#ifdef HAS_INSTALL_ENV
    /* This was tested on gwyn during development. */
    std::ifstream old_shadow("/target/etc/shadow");
    if(!old_shadow) {
        output_error("installfile:" + std::to_string(this->lineno()),
                     "rootpw: cannot open existing shadow file");
        return false;
    }

    std::stringstream shadow_stream;
    char shadow_line[200];
    /* Discard root. */
    old_shadow.getline(shadow_line, sizeof(shadow_line));
    assert(strncmp(shadow_line, "root", 4) == 0);

    /* Insert the new root line... */
    shadow_stream << root_line << std::endl;
    /* ...and copy the rest of the old shadow file. */
    while(old_shadow.getline(shadow_line, sizeof(shadow_line))) {
        shadow_stream << shadow_line << std::endl;
    }

    old_shadow.close();

    std::ofstream new_shadow("/target/etc/shadow", std::ios_base::trunc);
    if(!new_shadow) {
        output_error("installfile:" + std::to_string(this->lineno()),
                     "rootpw: cannot replace target shadow file");
        return false;
    }
    new_shadow << shadow_stream.str();
    return true;
#else
    return false;
#endif /* HAS_INSTALL_ENV */
}


Key *Username::parseFromData(const std::string &data, int lineno, int *errors,
                             int *warnings) {
    if(!is_valid_name(data.c_str())) {
        if(errors) *errors += 1;
        output_error("installfile:" + std::to_string(lineno),
                     "username: invalid username specified");
        return nullptr;
    }

    /* REQ: Runner.Validate.username.System */
    if(system_names.find(data) != system_names.end()) {
        if(errors) *errors += 1;
        output_error("installfile:" + std::to_string(lineno),
                     "username: " + data + " is a reserved system username");
        return nullptr;
    }

    return new Username(lineno, data);
}

bool Username::execute(ScriptOptions) const {
    return false;
}


Key *UserAlias::parseFromData(const std::string &data, int lineno, int *errors,
                              int *warnings) {
    /* REQ: Runner.Validate.useralias.Validity */
    const std::string::size_type sep = data.find_first_of(' ');
    if(sep == std::string::npos || data.length() == sep + 1) {
        if(errors) *errors += 1;
        output_error("installfile:" + std::to_string(lineno),
                     "useralias: alias is required",
                     "expected format is: useralias [username] [alias...]");
        return nullptr;
    }

    return new UserAlias(lineno, data.substr(0, sep), data.substr(sep + 1));
}

bool UserAlias::validate(ScriptOptions) const {
    return true;
}

bool UserAlias::execute(ScriptOptions) const {
    return false;
}


Key *UserPassphrase::parseFromData(const std::string &data, int lineno,
                                   int *errors, int *warnings) {
    /* REQ: Runner.Validate.userpw.Validity */
    const std::string::size_type sep = data.find_first_of(' ');
    if(sep == std::string::npos || data.length() == sep + 1) {
        if(errors) *errors += 1;
        output_error("installfile:" + std::to_string(lineno),
                     "userpw: passphrase is required",
                     "expected format is: userpw [username] [crypt...]");
        return nullptr;
    }

    std::string passphrase = data.substr(sep + 1);
    if(!string_is_crypt(passphrase, "userpw", lineno)) {
        if(errors) *errors += 1;
        return nullptr;
    }

    return new UserPassphrase(lineno, data.substr(0, sep), data.substr(sep + 1));
}

bool UserPassphrase::validate(ScriptOptions) const {
    /* If it's parseable, it's valid. */
    return true;
}

bool UserPassphrase::execute(ScriptOptions) const {
    return false;
}


Key *UserIcon::parseFromData(const std::string &data, int lineno, int *errors,
                             int *warnings) {
    /* REQ: Runner.Validate.usericon.Validity */
    const std::string::size_type sep = data.find_first_of(' ');
    if(sep == std::string::npos || data.length() == sep + 1) {
        if(errors) *errors += 1;
        output_error("installfile:" + std::to_string(lineno),
                     "usericon: icon is required",
                     "expected format is: usericon [username] [path|url]");
        return nullptr;
    }

    std::string icon_path = data.substr(sep + 1);
    if(icon_path[0] != '/' && !is_valid_url(icon_path)) {
        if(errors) *errors += 1;
        output_error("installfile:" + std::to_string(lineno),
                     "usericon: path must be absolute path or valid URL");
        return nullptr;
    }

    return new UserIcon(lineno, data.substr(0, sep), icon_path);
}

bool UserIcon::validate(ScriptOptions) const {
    /* TODO XXX: ensure URL is accessible */
    return true;
}

bool UserIcon::execute(ScriptOptions) const {
    return false;
}


Key *UserGroups::parseFromData(const std::string &data, int lineno,
                               int *errors, int *warnings) {
    /* REQ: Runner.Validate.usergroups.Validity */
    const std::string::size_type sep = data.find_first_of(' ');
    if(sep == std::string::npos || data.length() == sep + 1) {
        if(errors) *errors += 1;
        output_error("installfile:" + std::to_string(lineno),
                     "usergroups: at least one group is required",
                     "expected format is: usergroups [user] [group(,...)]");
        return nullptr;
    }

    std::set<std::string> group_set;
    char next_group[17];
    std::istringstream stream(data.substr(sep + 1));
    while(stream.getline(next_group, 17, ',')) {
        std::string group(next_group);
        /* REQ: Runner.Validate.usergroups.Group */
        if(system_groups.find(group) == system_groups.end()) {
            if(errors) *errors += 1;
            output_error("installfile:" + std::to_string(lineno),
                         "usergroups: group name '" + group + "' is invalid",
                         "group is not a recognised system group");
            return nullptr;
        }
        group_set.insert(group);
    }
    /* REQ: Runner.Validate.usergroups.Group */
    if(stream.fail() && !stream.eof()) {
        if(errors) *errors += 1;
        output_error("installfile:" + std::to_string(lineno),
                     "usergroups: group name exceeds maximum length",
                     "groups may only be 16 characters or less");
        return nullptr;
    }

    return new UserGroups(lineno, data.substr(0, sep), group_set);
}

bool UserGroups::validate(ScriptOptions) const {
    /* All validation is done in parsing stage */
    return true;
}

bool UserGroups::execute(ScriptOptions) const {
    return false;
}
