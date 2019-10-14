/*
 * disk.cc - Implementation of the Key classes for disk manipulation
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
#include <string>
#include <unistd.h>
#include "disk.hh"
#include "util/output.hh"

using namespace Horizon::Keys;

Key *Mount::parseFromData(const std::string &data, int lineno, int *errors,
                          int *warnings) {
    std::string dev, where, opt;
    std::string::size_type where_pos, opt_pos;
    bool any_failure = false;

    long spaces = std::count(data.cbegin(), data.cend(), ' ');
    if(spaces < 1 || spaces > 2) {
        if(errors) *errors += 1;
        /* Don't bother with any_failure, because this is immediately fatal. */
        output_error("installfile:" + std::to_string(lineno),
                     "mount: expected either 2 or 3 elements, got: " +
                     std::to_string(spaces), "");
        return nullptr;
    }

    where_pos = data.find_first_of(' ');
    opt_pos = data.find_first_of(' ', where_pos + 1);

    dev = data.substr(0, where_pos);
    where = data.substr(where_pos + 1, (opt_pos - where_pos - 1));
    if(opt_pos != std::string::npos && data.length() > opt_pos + 1) {
        opt = data.substr(opt_pos + 1);
    }

    if(dev.compare(0, 4, "/dev")) {
        if(errors) *errors += 1;
        any_failure = true;
        output_error("installfile:" + std::to_string(lineno),
                     "mount: element 1: expected device node",
                     "'" + dev + "' is not a valid device node");
    }

    if(where[0] != '/') {
        if(errors) *errors += 1;
        any_failure = true;
        output_error("installfile:" + std::to_string(lineno),
                     "mount: element 2: expected absolute path",
                     "'" + where + "' is not a valid absolute path");
    }

    if(any_failure) return nullptr;

    return new Mount(lineno, dev, where, opt);
}

bool Mount::validate(ScriptOptions options) const {
    /* We only validate if running in an Installation Environment. */
    if(!options.test(InstallEnvironment)) return true;

    /* XXX TODO: This will fail validation if the block device does not
     * already exist.  However, we must take in to account that block devices
     * may not yet exist during the script validation phase.  This check may
     * need to happen in Script::validate like the Uniqueness tests. */
    return(access(this->device().c_str(), F_OK) == 0);
}

bool Mount::execute(ScriptOptions options) const {
    const std::string actual_mount = "/target" + this->mountpoint();

    if(options.test(Simulate)) {
        output_info("installfile:" + std::to_string(this->lineno()),
                    "mount: mounting " + this->device() + " on " +
                    this->mountpoint());
        std::cout << "mount ";
        if(!this->options().empty()) {
            std::cout << "-o " << this->options() << " ";
        }
        std::cout << this->device() << " " << actual_mount << std::endl;
        return true;
    }

    /* mount */
    return true;
}
