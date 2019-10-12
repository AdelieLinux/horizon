/*
 * network.hh - Definition of the Key classes for network configuration
 * libhscript, the HorizonScript library for
 * Project Horizon
 *
 * Copyright (c) 2019 Adélie Linux and contributors.  All rights reserved.
 * This code is licensed under the AGPL 3.0 license, as noted in the
 * LICENSE-code file in the root directory of this repository.
 *
 * SPDX-License-Identifier: AGPL-3.0-only
 */

#ifndef __HSCRIPT_NETWORK_HH_
#define __HSCRIPT_NETWORK_HH_

#include "key.hh"
#include "util/output.hh"

namespace Horizon {
namespace Keys {

class Network : public BooleanKey {
private:
    Network(int _line, bool _value) : BooleanKey(_line, _value) {}
public:
    static Key *parseFromData(const std::string data, int lineno, int *errors,
                              int *warnings);
    bool execute(ScriptOptions) const override;
};

class NetAddress : public Key {
public:
    /*! Determines the type of address an interface will obtain. */
    enum AddressType {
        /*! DHCP address, obtained automatically from an addressing server. */
        DHCP,
        /*! IPv6 address automatically derived from router solicitation. */
        SLAAC,
        /*! Static address obtained from the netaddress key. */
        Static
    };
private:
    const std::string _iface;
    const AddressType _type;
    const std::string _address;
    const uint8_t _prefix;
    const std::string _gw;

    NetAddress(const int _line, const std::string _i, const AddressType _t,
               const std::string _a, const uint8_t _p, const std::string _g) :
        Key(_line), _iface(_i), _type(_t), _address(_a), _prefix(_p), _gw(_g)
    {}
public:
    static Key *parseFromData(const std::string data, int lineno, int *errors,
                              int *warnings);

    /*! Retrieve the interface to which this 'netaddress' key is associated. */
    const std::string iface() const { return this->_iface; }
    /*! Retrieve the address type of this 'netadress' key. */
    const AddressType type() const { return this->_type; }
    /*! Retrieve the static address, if any. */
    const std::string address() const { return this->_address; }
    /*! Retreive the prefix length for the static address. */
    const uint8_t prefix() const { return this->_prefix; }
    /*! Retrieve the gateway, if any. */
    const std::string gateway() const { return this->_gw; }

    bool validate(ScriptOptions) const override;
    bool execute(ScriptOptions) const override;
};

class Nameserver : public Key {
};

class NetSSID : public Key {
};

}
}

#endif /* !__HSCRIPT_NETWORK_HH */
