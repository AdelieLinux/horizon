/*
 * meta.cc - Implementation of the Key classes for system metadata
 * libhscript, the HorizonScript library for
 * Project Horizon
 *
 * Copyright (c) 2019 Adélie Linux and contributors.  All rights reserved.
 * This code is licensed under the AGPL 3.0 license, as noted in the
 * LICENSE-code file in the root directory of this repository.
 *
 * SPDX-License-Identifier: AGPL-3.0-only
 */

#include <regex>
#include "meta.hh"
#include "util/output.hh"

using namespace Horizon::Keys;

Key *Hostname::parseFromData(const std::string data, int lineno, int *errors,
                             int *warnings) {
    std::regex valid_re("[A-Za-z0-9-_.]*");
    if(!std::regex_match(data, valid_re)) {
        if(errors) *errors += 1;
        output_error("installfile:" + std::to_string(lineno),
                     "hostname: expected machine or DNS name",
                     "'" + data + "' is not a valid hostname");
        return nullptr;
    }
    return new Hostname(lineno, data);
}

bool Hostname::validate(ScriptOptions) const {
    /* Validate that the name is a valid machine or DNS name */
    bool any_failure = false;
    std::string::size_type last_dot = 0, next_dot = 0;

    if(!isalpha(this->_value[0])) {
        any_failure = true;
        output_error("installfile:" + std::to_string(this->lineno()),
                     "hostname: must start with alphabetical character");
    }

    if(this->_value.size() > 320) {
        any_failure = true;
        output_error("installfile:" + std::to_string(this->lineno()),
                     "hostname: value too long",
                     "valid host names must be less than 320 characters");
    }

    do {
        next_dot = this->_value.find_first_of('.', next_dot + 1);
        if(next_dot == std::string::npos) {
            next_dot = this->_value.size();
        }
        if(next_dot - last_dot > 64) {
            any_failure = true;
            output_error("installfile:" + std::to_string(this->lineno()),
                         "hostname: component too long",
                         "each component must be less than 64 characters");
        }
    } while(next_dot != this->_value.size());

    return any_failure;
}

bool Hostname::execute(ScriptOptions) const {
    /* Write the hostname to /etc/hostname in the target environment. */
    return false;
}


Key *PkgInstall::parseFromData(const std::string data, int lineno, int *errors,
                               int *warnings) {
    std::regex valid_pkg("[0-9A-Za-z+_.-]*((>?<|[<>]?=|[~>])[0-9A-Za-z-_.]+)?");
    std::string next_pkg;
    std::istringstream stream(data);
    std::set<std::string> all_pkgs;

    while(stream >> next_pkg) {
        if(!std::regex_match(next_pkg, valid_pkg)) {
            if(errors) *errors += 1;
            output_error("installfile:" + std::to_string(lineno),
                         "pkginstall: expected package name",
                         "'" + next_pkg + "' is not a valid package or atom");
            return nullptr;
        }
        if(all_pkgs.find(next_pkg) != all_pkgs.end()) {
            if(warnings) *warnings += 1;
            output_warning("installfile:" + std::to_string(lineno),
                           "pkginstall: package '" + next_pkg +
                           "' is already in the target package set");
            continue;
        }
        all_pkgs.insert(next_pkg);
    }
    return new PkgInstall(lineno, all_pkgs);
}
