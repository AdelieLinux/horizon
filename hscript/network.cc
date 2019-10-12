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

#include <algorithm>
#include "network.hh"
#include "util/output.hh"

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
    long elements = std::count(data.cbegin(), data.cend(), ' ') + 1;
    std::string::size_type type_pos, addr_pos, prefix_pos, gw_pos;
    std::string iface, type, addr, prefix, gw;

    if(elements < 2) {
        if(errors) *errors += 1;
        output_error("installfile:" + std::to_string(lineno),
                     "netaddress: missing address type",
                     "one of 'dhcp', 'slaac', 'static' required");
        return nullptr;
    }

    type_pos = data.find_first_of(' ');
    iface = data.substr(0, type_pos);
    /* theory: addr_pos could be npos, but that means 'read to end' anyway */
    addr_pos = data.find_first_of(' ', type_pos + 1);
    type = data.substr(type_pos + 1, (addr_pos - type_pos - 1));

    /* ensure type is lower-cased, in case someone types 'DHCP' or 'SLAAC' */
    std::transform(type.begin(), type.end(), type.begin(), ::tolower);

    if(!type.compare("dhcp")) {
        if(elements > 2) {
            if(errors) *errors += 1;
            output_error("installfile:" + std::to_string(lineno),
                         "netaddress: address type 'dhcp' does not "
                         "accept further elements");
            return nullptr;
        }
        return new NetAddress(lineno, iface, AddressType::DHCP, "", 0, "");
    } else if(!type.compare("slaac")) {
        if(elements > 2) {
            if(errors) *errors += 1;
            output_error("installfile:" + std::to_string(lineno),
                         "netaddress: address type 'slaac' does not "
                         "accept further elements");
            return nullptr;
        }
        return new NetAddress(lineno, iface, AddressType::SLAAC, "", 0, "");
    } else if(type.compare("static")) {
        if(errors) *errors += 1;
        output_error("installfile:" + std::to_string(lineno),
                     "netaddress: invalid address type '" + type + "'",
                     "one of 'dhcp', 'slaac', 'static' required");
        return nullptr;
    }

    /* static address */
    if(elements < 4) {
        if(errors) *errors += 1;
        output_error("installfile:" + std::to_string(lineno),
                     "netaddress: address type 'static' requires at least "
                     "an IP address and prefix length");
        return nullptr;
    }

    if(elements > 5) {
        if(errors) *errors += 1;
        output_error("installfile:" + std::to_string(lineno),
                     "netaddress: too many elements to address type 'static'");
        return nullptr;
    }

    return nullptr;
}

bool NetAddress::validate(ScriptOptions) const {
    /* possible to validate an address in the Installation Environment? */
    return true;
}

bool NetAddress::execute(ScriptOptions) const {
    return false;
}
