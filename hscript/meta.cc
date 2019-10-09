/*
 * metadata.cc - Implementation of the Key classes for system metadata
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
    std::regex valid_re("[A-Za-z0-9.]*");
    if(!std::regex_match(data, valid_re)) {
        if(errors) *errors += 1;
        output_error("installfile:" + std::to_string(lineno),
                     "hostname: expected machine or DNS name",
                     "'" + data + "' is not a valid hostname");
        return nullptr;
    }
    return new Hostname(lineno, data);
}

bool Hostname::validate() const {
    /* Validate that the name is a valid machine or DNS name */
    return false;
}

bool Hostname::execute() const {
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
