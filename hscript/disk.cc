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
#include "disk.hh"
#include "util/output.hh"

using namespace Horizon::Keys;

Key *Mount::parseFromData(const std::string data, int lineno, int *errors,
                          int *warnings) {
    std::string dev, where, opt;
    long spaces = std::count(data.cbegin(), data.cend(), ' ');
    if(spaces < 1 || spaces > 2) {
        if(errors) *errors += 1;
        output_error("installfile:" + std::to_string(lineno),
                     "mount: expected either 2 or 3 elements, got: " +
                     std::to_string(spaces), "");
        return nullptr;
    }
    return new Mount(lineno, dev, where, opt);
}

bool Mount::validate() const {
    return false;
}

bool Mount::execute() const {
    return false;
}
