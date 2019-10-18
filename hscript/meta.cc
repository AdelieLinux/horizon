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

#include <assert.h>
#include <fstream>
#include <regex>
#include <unistd.h>
#include "meta.hh"
#include "util/output.hh"

using namespace Horizon::Keys;

Key *Hostname::parseFromData(const std::string &data, int lineno, int *errors,
                             int *) {
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
        last_dot = next_dot;
    } while(next_dot != this->_value.size());

    return !any_failure;
}

bool Hostname::execute(ScriptOptions opts) const {
    /* Set the hostname of the target computer */
    std::string actual;
    std::string::size_type dot = this->_value.find_first_of('.');

    if(this->_value.size() > 64) {
        /* Linux has a nodename limit of 64 characters.
         * That's fine, because we have a limit of 64 chars per segment.
         * Assuming a dot is present, just chop at the first dot. */
        assert(dot <= 64);
        actual = this->_value.substr(0, dot);
    } else {
        actual = this->_value;
    }

    /* Runner.Execute.hostname. */
    output_info("installfile:" + std::to_string(this->lineno()),
                "hostname: set hostname to '" + actual + "'");
    if(opts.test(Simulate)) {
        std::cout << "hostname " << actual << std::endl;
    } else { /* LCOV_EXCL_START */
        if(sethostname(actual.c_str(), actual.size()) == -1) {
            output_error("installfile:" + std::to_string(this->lineno()),
                         "hostname: failed to set host name",
                         std::string(strerror(errno)));
            return false;
        }
    } /* LCOV_EXCL_STOP */

    /* Runner.Execute.hostname.Write. */
    output_info("installfile:" + std::to_string(this->lineno()),
                "hostname: write '" + actual + "' to /etc/hostname");
    if(opts.test(Simulate)) {
        std::cout << "printf '%s' " << actual << " > /target/etc/hostname"
                  << std::endl;
    } else { /* LCOV_EXCL_START */
        std::ofstream hostname_f("/target/etc/hostname");
        if(!hostname_f) {
            output_error("installfile:" + std::to_string(this->lineno()),
                         "hostname: could not open /etc/hostname for writing");
            return false;
        }
        hostname_f << actual;
    } /* LCOV_EXCL_STOP */

    /* The second condition ensures that it isn't a single dot that simply
     * terminates the nodename. */
    if(dot != std::string::npos && this->_value.length() > dot + 1) {
        const std::string domain(this->_value.substr(dot + 1));
        output_info("installfile:" + std::to_string(this->lineno()),
                    "hostname: set domain name '" + domain + "'");
        if(opts.test(Simulate)) {
            std::cout << "printf 'dns_domain_lo=\"" << domain
                      << "\"\\" << "n' >> /target/etc/conf.d/net" << std::endl;
        } else {
            std::ofstream net_conf_f("/target/etc/conf.d/net");
            if(!net_conf_f) {
                output_error("installfile:" + std::to_string(this->lineno()),
                             "hostname: could not open /etc/conf.d/net for "
                             "writing");
                return false;
            }
            net_conf_f << "dns_domain_lo\"" << domain << "\"" << std::endl;
        }
    }

    return true;
}


static std::regex valid_pkg("[0-9A-Za-z+_.-]*((>?<|[<>]?=|[~>])[0-9A-Za-z-_.]+)?");


Key *PkgInstall::parseFromData(const std::string &data, int lineno, int *errors,
                               int *warnings) {
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


/* LCOV_EXCL_START */
bool PkgInstall::validate(ScriptOptions) const {
    /* Any validation errors would have occurred above. */
    return true;
}


bool PkgInstall::execute(ScriptOptions) const {
    /* Package installation is handled in Script::execute. */
    return true;
}
/* LCOV_EXCL_STOP */


Key *Repository::parseFromData(const std::string &data, int lineno, int *errors,
                               int *) {
    if(data.empty() || (data[0] != '/' && data.compare(0, 4, "http"))) {
        if(errors) *errors += 1;
        output_error("installfile:" + std::to_string(lineno),
                     "repository: must be absolute path or HTTP(S) URL");
        return nullptr;
    }
    return new Repository(lineno, data);
}

bool Repository::validate(ScriptOptions) const {
    /* TODO XXX: Ensure URL is accessible if networking is available */
    return true;
}

bool Repository::execute(ScriptOptions opts) const {
    /* Runner.Execute.repository. */
    output_info("installfile:" + std::to_string(this->lineno()),
                "repository: write '" + this->value() +
                "' to /etc/apk/repositories");
    if(opts.test(Simulate)) {
        std::cout << "echo '" << this->value() <<
                     "' >> /target/etc/apk/repositories" << std::endl;
        return true;
    }

    /* LCOV_EXCL_START */
    std::ofstream repo_f("/target/etc/apk/repositories",
                         std::ios_base::ate);
    if(!repo_f) {
        output_error("installfile:" + std::to_string(this->lineno()),
                     "repository: could not open /etc/apk/repositories "
                     "for writing");
        return false;
    }

    repo_f << this->value() << std::endl;

    return true;
    /* LCOV_EXCL_STOP */
}
