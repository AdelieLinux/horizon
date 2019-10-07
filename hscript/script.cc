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

#include <fstream>
#include <iostream>
#include "script.hh"
#include "disk.hh"
#include "meta.hh"
#include "network.hh"
#include "user.hh"

#include "util/output.hh"

#define LINE_MAX 512

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

const Script *Script::load(std::string path, ScriptOptions opts) {
    std::ifstream file(path);
    if(!file) {
        output_error(path, "Cannot open installfile", "",
                     (opts.test(Pretty)));
        return nullptr;
    }

    return Script::load(file, opts);
}

/*! Determines if the specified +key+ has been defined in this version of
 * HorizonScript.
 */
bool is_key(std::string key) {
    return true;
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

const Script *Script::load(std::istream &sstream, ScriptOptions opts) {
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

        if(!is_key(key)) {
            /* Invalid key */
            if(opts.test(StrictMode)) {
                PARSER_ERROR("key '" + key + "' is not defined")
            } else {
                PARSER_WARNING("key '" + key + "' is not defined")
            }
        }
    }

    output_message("parser", "0", "installfile",
                   std::to_string(errors) + " error(s), " +
                   std::to_string(warnings) + " warning(s).",
                   "", opts.test(Pretty));

    if(errors > 0) {
        delete the_script;
        return nullptr;
    }

    if(sstream.fail() && !sstream.eof()) {
        output_error("installfile:" + std::to_string(lineno + 1),
                     "line exceeds maximum length",
                     "Maximum length for line is " + std::to_string(LINE_MAX),
                     (opts.test(Pretty)));
        delete the_script;
        return nullptr;
    }

    if(sstream.bad() && !sstream.eof()) {
        output_error("installfile:" + std::to_string(lineno),
                     "I/O error reading installfile", "",
                     (opts.test(Pretty)));
    }

    return the_script;
}

bool Script::validate() const {
    return false;
}

}
