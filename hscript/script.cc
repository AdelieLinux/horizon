/*
 * script.hh - Implementation of the Script class
 * libhscript, the HorizonScript library for
 * Project Horizon
 *
 * Copyright (c) 2019 Adélie Linux and contributors.  All rights reserved.
 * This code is licensed under the AGPL 3.0 license, as noted in the
 * LICENSE-code file in the root directory of this repository.
 *
 * SPDX-License-Identifier: AGPL-3.0-only
 */

#include <algorithm>
#include <fstream>
#include <iostream>
#include <map>

#include "script.hh"
#include "disk.hh"
#include "meta.hh"
#include "network.hh"
#include "user.hh"

#include "util/output.hh"

#define LINE_MAX 512


typedef Horizon::Keys::Key *(*key_parse_fn)(std::string, int, int*, int*);

const std::map<std::string, key_parse_fn> valid_keys = {
    {"network", &Horizon::Keys::Network::parseFromData},
    {"hostname", &Horizon::Keys::Hostname::parseFromData},
    {"pkginstall", &Horizon::Keys::PkgInstall::parseFromData},
    {"rootpw", &Horizon::Keys::RootPassphrase::parseFromData},

    {"language", &Horizon::Keys::Language::parseFromData},
    {"keymap", &Horizon::Keys::Keymap::parseFromData},
    {"firmware", &Horizon::Keys::Firmware::parseFromData},
    {"timezone", &Horizon::Keys::Timezone::parseFromData},
    {"repository", &Horizon::Keys::Repository::parseFromData},
    {"signingkey", &Horizon::Keys::SigningKey::parseFromData},

    {"netaddress", &Horizon::Keys::NetAddress::parseFromData},
    {"nameserver", &Horizon::Keys::Nameserver::parseFromData},
    {"netssid", &Horizon::Keys::NetSSID::parseFromData},

    {"username", &Horizon::Keys::Username::parseFromData},
    {"useralias", &Horizon::Keys::UserAlias::parseFromData},
    {"userpw", &Horizon::Keys::UserPassphrase::parseFromData},
    {"usericon", &Horizon::Keys::UserIcon::parseFromData},
    {"usergroups", &Horizon::Keys::UserGroups::parseFromData},

    {"diskid", &Horizon::Keys::DiskId::parseFromData},
    {"disklabel", &Horizon::Keys::DiskLabel::parseFromData},
    {"partition", &Horizon::Keys::Partition::parseFromData},
    {"lvm_pv", &Horizon::Keys::LVMPhysical::parseFromData},
    {"lvm_vg", &Horizon::Keys::LVMGroup::parseFromData},
    {"lvm_lv", &Horizon::Keys::LVMVolume::parseFromData},
    {"encrypt", &Horizon::Keys::Encrypt::parseFromData},
    {"fs", &Horizon::Keys::Filesystem::parseFromData},
    {"mount", &Horizon::Keys::Mount::parseFromData}
};


namespace Horizon {

struct Script::ScriptPrivate {
    /*! Determines whether or not to enable networking. */
    std::unique_ptr<Horizon::Keys::Network> network;
    /*! The target system's hostname. */
    std::unique_ptr<Horizon::Keys::Hostname> hostname;
    /*! The packages to install to the target system. */
    std::vector<std::string> packages;
    /*! The root shadow line. */
    std::unique_ptr<Horizon::Keys::RootPassphrase> rootpw;
    /*! Target system's mountpoints. */
    std::vector< std::unique_ptr<Horizon::Keys::Mount> > mounts;
};

Script::Script() {
    internal = new ScriptPrivate;
}

const Script *Script::load(const std::string path, const ScriptOptions opts) {
    std::ifstream file(path);
    if(!file) {
        output_error(path, "Cannot open installfile", "",
                     (opts.test(Pretty)));
        return nullptr;
    }

    return Script::load(file, opts);
}

#define PARSER_ERROR(err_str) \
    errors++;\
    output_error("installfile:" + std::to_string(lineno),\
                 err_str, "", (opts.test(Pretty)));\
    if(!opts.test(ScriptOptionFlags::KeepGoing)) {\
        break;\
    }

#define PARSER_WARNING(warn_str) \
    warnings++;\
    output_warning("installfile:" + std::to_string(lineno),\
                   warn_str, "", (opts.test(Pretty)));

const Script *Script::load(std::istream &sstream, const ScriptOptions opts) {
    using namespace Horizon::Keys;
    Script *the_script = new Script;

    int lineno = 0;
    char nextline[LINE_MAX];
    const std::string delim(" \t");
    int errors = 0, warnings = 0;

    while(sstream.getline(nextline, sizeof(nextline))) {
        lineno++;
        if(nextline[0] == '#') {
            /* This is a comment line; ignore it. */
            continue;
        }

        const std::string line(nextline);
        std::string key;
        std::string::size_type start, key_end, value_begin;
        start = line.find_first_not_of(delim);
        if(start == std::string::npos) {
            /* This is a blank line; ignore it. */
            continue;
        }

        key_end = line.find_first_of(delim, start);
        value_begin = line.find_first_not_of(delim, key_end);
        key = line.substr(start, key_end);
        if(key_end == std::string::npos || value_begin == std::string::npos) {
            /* Key without value */
            PARSER_ERROR("key '" + key + "' has no value")
        }

        /* Normalise key to lower-case */
        std::transform(key.begin(), key.end(), key.begin(), ::tolower);

        if(valid_keys.find(key) == valid_keys.end()) {
            /* Invalid key */
            if(opts.test(StrictMode)) {
                PARSER_ERROR("key '" + key + "' is not defined")
            } else {
                PARSER_WARNING("key '" + key + "' is not defined")
            }
            continue;
        }

        Key *key_obj = valid_keys.at(key)(line.substr(value_begin), lineno,
                                          &errors, &warnings);
        if(!key_obj) {
            PARSER_ERROR("value for key '" + key + "' was invalid")
        }
    }

    if(sstream.fail() && !sstream.eof()) {
        output_error("installfile:" + std::to_string(lineno + 1),
                     "line exceeds maximum length",
                     "Maximum line length is " + std::to_string(LINE_MAX),
                     (opts.test(Pretty)));
        errors++;
    }

    if(sstream.bad() && !sstream.eof()) {
        output_error("installfile:" + std::to_string(lineno),
                     "I/O error while reading installfile", "",
                     (opts.test(Pretty)));
        errors++;
    }

    output_message("parser", "0", "installfile",
                   std::to_string(errors) + " error(s), " +
                   std::to_string(warnings) + " warning(s).",
                   "", opts.test(Pretty));

    if(errors > 0) {
        delete the_script;
        return nullptr;
    } else {
        return the_script;
    }
}

bool Script::validate() const {
    return false;
}

}
