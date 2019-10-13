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

#include "user.hh"
#include "util/output.hh"

using namespace Horizon::Keys;

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

bool RootPassphrase::execute(ScriptOptions) const {
    return false;
}
