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
    static Key *parseFromData(const std::string &data, int lineno, int *errors,
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

    NetAddress(const int _line, const std::string &_i, const AddressType &_t,
               const std::string &_a, const uint8_t _p, const std::string &_g) :
        Key(_line), _iface(_i), _type(_t), _address(_a), _prefix(_p), _gw(_g)
    {}
public:
    static Key *parseFromData(const std::string &data, int lineno, int *errors,
                              int *warnings);

    /*! Retrieve the interface to which this 'netaddress' key is associated. */
    const std::string iface() const { return this->_iface; }
    /*! Retrieve the address type of this 'netadress' key. */
    AddressType type() const { return this->_type; }
    /*! Retrieve the static address, if any. */
    const std::string address() const { return this->_address; }
    /*! Retreive the prefix length for the static address. */
    uint8_t prefix() const { return this->_prefix; }
    /*! Retrieve the gateway, if any. */
    const std::string gateway() const { return this->_gw; }

    bool validate(ScriptOptions) const override;
    bool execute(ScriptOptions) const override;
};

class Nameserver : public StringKey {
private:
    Nameserver(int _line, const std::string &ns) : StringKey(_line, ns) {}
public:
    static Key *parseFromData(const std::string &, int, int *, int *);
    bool execute(ScriptOptions) const override;
};

class NetSSID : public Key {
public:
    /*! The type of security used by the SSID. */
    enum SecurityType {
        None,
        WEP,
        WPA
    };
private:
    const std::string _iface;
    const std::string _ssid;
    const SecurityType _sec;
    const std::string _pw;

    NetSSID(int _line, const std::string &_if, const std::string &_s,
            SecurityType _t, const std::string &_p) : Key(_line), _iface(_if),
        _ssid(_s), _sec(_t), _pw(_p) {}
public:
    static Key *parseFromData(const std::string &data, int lineno, int *errors,
                              int *warnings);

    /*! Retrieve the interface to which this 'netssid' key is associated. */
    const std::string iface() const { return this->_iface; }
    /*! Retrieve the named SSID for this 'netssid' key. */
    const std::string ssid() const { return this->_ssid; }
    /*! Retrieve the security type of this 'netssid' key. */
    SecurityType type() const { return this->_sec; }
    /*! Retrieve the passphrase for this 'netssid' key.
     * @note Only valid if type() is not None.
     */
    const std::string passphrase() const { return this->_pw; }

    bool validate(ScriptOptions) const override;
    bool execute(ScriptOptions) const override;
};

}
}

#endif /* !__HSCRIPT_NETWORK_HH */
