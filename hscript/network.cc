/*
 * network.cc - Implementation of the network Key classes
 * libhscript, the HorizonScript library for
 * Project Horizon
 *
 * Copyright (c) 2019 Adélie Linux and contributors.  All rights reserved.
 * This code is licensed under the AGPL 3.0 license, as noted in the
 * LICENSE-code file in the root directory of this repository.
 *
 * SPDX-License-Identifier: AGPL-3.0-only
 */

#include "network.hh"

using namespace Horizon::Keys;

Key *Network::parseFromData(const std::string data, int lineno, int *errors,
                            int *warnings) {
    bool value;
    if(!BooleanKey::parse(data, &value)) {
        output_error("installfile:" + std::to_string(lineno),
                     "network: expected 'true' or 'false'",
                     "'" + data + "' is not a valid Boolean value");
        if(errors) *errors += 1;
        return nullptr;
    }
    return new Network(value);
}

bool Network::execute() {
    return false;
}
