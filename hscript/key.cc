/*
 * key.cc - Implementation of common routines for the base Key classes
 * libhscript, the HorizonScript library for
 * Project Horizon
 *
 * Copyright (c) 2019-2020 Adélie Linux and contributors.  All rights reserved.
 * This code is licensed under the AGPL 3.0 license, as noted in the
 * LICENSE-code file in the root directory of this repository.
 *
 * SPDX-License-Identifier: AGPL-3.0-only
 */

#include <algorithm>
#include "key.hh"
#include "util/output.hh"


Horizon::Keys::Key::~Key() {
}

bool Horizon::Keys::BooleanKey::parse(const std::string &what,
                                      const ScriptLocation &where,
                                      const std::string &key, bool *out) {
    std::string lower(what.size(), 0);
    std::transform(what.begin(), what.end(), lower.begin(), ::tolower);

    if(lower == "true" || lower == "yes" || lower == "1") {
        *out = true;
    } else if(lower == "false" || lower == "no" || lower == "0") {
        *out = false;
    } else {
        output_error(where, key + ": expected 'true' or 'false'",
                     "'" + what + "' is not a valid Boolean value");
        return false;
    }
    return true;
}

bool Horizon::Keys::BooleanKey::validate() const {
    /* Key will fail init if it is not valid, so this is always a no-op. */
    return true;
}

bool Horizon::Keys::StringKey::validate() const {
    /* Key will fail init if it is not valid, so this is always a no-op. */
    return true;
}
