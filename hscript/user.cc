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


Key *RootPassphrase::parseFromData(const std::string &data, int lineno,
                                   int *errors, int *warnings) {
    if(data.size() < 5 || data[0] != '$' || (data[1] != '2' && data[1] != '6')
            || data[2] != '$') {
        if(errors) *errors += 1;
        output_error("installfile:" + std::to_string(lineno),
                     "rootpw: value is not a crypt-style encrypted passphrase");
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

    if(options.test(Simulate)) {
        std::cout << "(printf '" << root_line << "\\" << "n'; "
                  << "cat /target/etc/shadow | sed '1d') > /tmp/shadow"
                  << std::endl
                  << "mv /tmp/shadow /target/etc/shadow" << std::endl
                  << "chown root:shadow /target/etc/shadow" << std::endl
                  << "chmod 640 /target/etc/shadow" << std::endl;
        return true;
    }

    /* LCOV_EXCL_START */
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
    /* LCOV_EXCL_STOP */
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
    return nullptr;
}

bool UserPassphrase::validate(ScriptOptions) const {
    return false;
}

bool UserPassphrase::execute(ScriptOptions) const {
    return false;
}


Key *UserIcon::parseFromData(const std::string &data, int lineno, int *errors,
                             int *warnings) {
    return nullptr;
}

bool UserIcon::validate(ScriptOptions) const {
    return false;
}

bool UserIcon::execute(ScriptOptions) const {
    return false;
}


Key *UserGroups::parseFromData(const std::string &data, int lineno,
                               int *errors, int *warnings) {
    return nullptr;
}

bool UserGroups::validate(ScriptOptions) const {
    return false;
}

bool UserGroups::execute(ScriptOptions) const {
    return false;
}
