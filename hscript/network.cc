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
#include <arpa/inet.h>
#include "network.hh"
#include "util/output.hh"

using namespace Horizon::Keys;

Key *Network::parseFromData(const std::string &data, int lineno, int *errors,
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

Key *NetAddress::parseFromData(const std::string &data, int lineno, int *errors,
                               int *warnings) {
    long elements = std::count(data.cbegin(), data.cend(), ' ') + 1;
    std::string::size_type type_pos, addr_pos, prefix_pos, gw_pos, next_end;
    std::string iface, type, addr, prefix, gw;
    int real_prefix;
    char addr_buf[16];

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
    if(addr_pos == std::string::npos) next_end = std::string::npos;
    else next_end = addr_pos - type_pos - 1;
    type = data.substr(type_pos + 1, next_end);

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

    prefix_pos = data.find_first_of(' ', addr_pos + 1);
    addr = data.substr(addr_pos + 1, (prefix_pos - addr_pos - 1));
    gw_pos = data.find_first_of(' ', prefix_pos + 1);
    if(gw_pos == std::string::npos) next_end = std::string::npos;
    else next_end = gw_pos - prefix_pos - 1;
    prefix = data.substr(prefix_pos + 1, next_end);
    if(gw_pos != std::string::npos) {
        gw = data.substr(gw_pos + 1);
    }

    if(addr.find(':') != std::string::npos) {
        /* IPv6 */
        if(::inet_pton(AF_INET6, addr.c_str(), &addr_buf) != 1) {
            if(errors) *errors += 1;
            output_error("installfile:" + std::to_string(lineno),
                         "netaddress: '" + addr + "' is not a valid IPv6 address",
                         "hint: a ':' was found, indicating this address is IPv6");
            return nullptr;
        }

        try {
            real_prefix = std::stoi(prefix);
        } catch(const std::exception &) {
            if(errors) *errors += 1;
            output_error("installfile:" + std::to_string(lineno),
                         "netaddress: prefix length is not a number",
                         "prefix must be a decimal value between 1 and 128");
            return nullptr;
        }

        if(real_prefix < 1 || real_prefix > 128) {
            if(errors) *errors += 1;
            output_error("installfile:" + std::to_string(lineno),
                         "netaddress: invalid IPv6 prefix length: " + prefix,
                         "prefix must be a decimal value between 1 and 128");
            return nullptr;
        }

        if(gw.size() > 0 &&
           ::inet_pton(AF_INET6, gw.c_str(), &addr_buf) != 1) {
            if(errors) *errors += 1;
            output_error("installfile:" + std::to_string(lineno),
                         "netaddress: '" + gw +
                         "' is not a valid IPv6 gateway",
                         "an IPv6 address must have an IPv6 gateway");
            return nullptr;
        }

        return new NetAddress(lineno, iface, AddressType::Static, addr,
                              static_cast<uint8_t>(real_prefix), gw);
    } else if(addr.find('.') != std::string::npos) {
        /* IPv4 */
        if(::inet_pton(AF_INET, addr.c_str(), &addr_buf) != 1) {
            if(errors) *errors += 1;
            output_error("installfile:" + std::to_string(lineno),
                         "netaddress: '" + addr + "' is not a valid IPv4 address");
            return nullptr;
        }

        /* There are two kinds of prefixes for IPv4: prefix length, like IPv6,
         * and mask, which is the old style used by i.e. Windows. */
        if(::inet_pton(AF_INET, prefix.c_str(), &addr_buf) != 1) {
            try {
                real_prefix = std::stoi(prefix);
            } catch(const std::exception &) {
                if(errors) *errors += 1;
                output_error("installfile:" + std::to_string(lineno),
                             "netaddress: can't parse prefix length/mask",
                             "a network mask or prefix length is required");
                return nullptr;
            }
        } else {
            uint32_t tmp;
            memcpy(&tmp, addr_buf, 4);
            tmp = ntohl(tmp);
            real_prefix = 1;
            while((tmp <<= 1) & 0x80000000) {
                real_prefix++;
            }
        }

        if(real_prefix < 1 || real_prefix > 32) {
            if(errors) *errors += 1;
            output_error("installfile:" + std::to_string(lineno),
                         "netaddress: invalid IPv4 prefix length: " + prefix,
                         "prefix must be between 1 and 32");
            return nullptr;
        }

        if(gw.size() > 0 &&
           ::inet_pton(AF_INET, gw.c_str(), &addr_buf) != 1) {
            if(errors) *errors += 1;
            output_error("installfile:" + std::to_string(lineno),
                         "netaddress: '" + gw +
                         "' is not a valid IPv4 gateway",
                         "an IPv4 address must have an IPv4 gateway");
            return nullptr;
        }

        return new NetAddress(lineno, iface, AddressType::Static, addr,
                              static_cast<uint8_t>(real_prefix), gw);
    } else {
        /* IPvBad */
        if(errors) *errors += 1;
        output_error("installfile:" + std::to_string(lineno),
                     "netaddress: invalid address of unknown type",
                     "an IPv4 or IPv6 address is required");
        return nullptr;
    }
}

bool NetAddress::validate(ScriptOptions) const {
    /* possible to validate an address in the Installation Environment? */
    return true;
}

bool NetAddress::execute(ScriptOptions) const {
    return false;
}

Key *NetSSID::parseFromData(const std::string &data, int lineno, int *errors,
                            int *warnings) {
    std::string iface, ssid, passphrase;
    return new NetSSID(lineno, iface, ssid, SecurityType::None, passphrase);
}

bool NetSSID::validate(ScriptOptions options) const {
    /* Runner.Validate.network.netssid.Interface */
    if(options.test(InstallEnvironment)) {
        return false;
    }
    return true;
}

bool NetSSID::execute(ScriptOptions) const {
    return false;
}
