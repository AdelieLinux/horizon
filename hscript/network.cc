/*
 * network.cc - Implementation of the Key classes for network configuration
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
    if(!BooleanKey::parse(data, "installfile:" + std::to_string(lineno),
                          "network", &value)) {
        if(errors) *errors += 1;
        return nullptr;
    }
    return new Network(lineno, value);
}

bool Network::execute(ScriptOptions) const {
    return false;
}

Key *NetAddress::parseFromData(const std::string data, int lineno, int *errors,
                               int *warnings) {
    return nullptr;
}

bool NetAddress::validate(ScriptOptions) const {
    return false;
}

bool NetAddress::execute(ScriptOptions) const {
    return false;
}
