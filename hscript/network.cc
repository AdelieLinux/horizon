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
#include <arpa/inet.h>          /* inet_pton */
#include <cstring>              /* memcpy */
#include <fstream>              /* ofstream for Net write */
#ifdef HAS_INSTALL_ENV
#   include <linux/wireless.h>     /* struct iwreq */
#   include <string.h>             /* strerror */
#   include <sys/ioctl.h>          /* ioctl, ioctl numbers */
#   include <unistd.h>             /* close */
#else
/*! The size of Linux interface names. */
#   define IFNAMSIZ 16
#endif
#include "network.hh"
#include "util/net.hh"
#include "util/output.hh"

using namespace Horizon::Keys;


Key *Network::parseFromData(const std::string &data, int lineno, int *errors,
                            int *) {
    bool value;
    if(!BooleanKey::parse(data, "installfile:" + std::to_string(lineno),
                          "network", &value)) {
        if(errors) *errors += 1;
        return nullptr;
    }
    return new Network(lineno, value);
}

bool Network::execute(ScriptOptions) const {
    /* The network key, by itself, does nothing. */
    return true;
}


Key *NetAddress::parseFromData(const std::string &data, int lineno, int *errors,
                               int *) {
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
    if(iface.length() > IFNAMSIZ) {
        if(errors) *errors += 1;
        output_error("installfile:" + std::to_string(lineno),
                     "netaddress: interface name '" + iface + "' is invalid",
                     "interface names must be 16 characters or less");
        return nullptr;
    }
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
        real_prefix = subnet_mask_to_cidr(prefix.c_str());
        if(real_prefix < 1) {
            try {
                real_prefix = std::stoi(prefix);
            } catch(const std::exception &) {
                if(errors) *errors += 1;
                output_error("installfile:" + std::to_string(lineno),
                             "netaddress: can't parse prefix length/mask",
                             "a network mask or prefix length is required");
                return nullptr;
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

bool NetAddress::validate(ScriptOptions opts) const {
    if(!opts.test(InstallEnvironment)) {
        return true;
    }

#ifdef HAS_INSTALL_ENV
    /* Retrieving the index is always valid, and is not even privileged. */
    struct ifreq request;
    int my_sock = ::socket(AF_INET, SOCK_STREAM, 0);
    if(my_sock == -1) {
        output_error("installfile:" + std::to_string(this->lineno()),
                     "netaddress: can't open socket", ::strerror(errno));
        return false;
    }
    memset(&request, 0, sizeof(request));
    memcpy(&request.ifr_name, _iface.c_str(), _iface.size());
    errno = 0;
    if(ioctl(my_sock, SIOCGIFFLAGS, &request) == -1) {
        if(errno == ENODEV) {
            output_warning("installfile:" + std::to_string(this->lineno()),
                           "netaddress: specified interface does not exist");
            return true;
        }
        output_error("installfile:" + std::to_string(this->lineno()),
                     "netaddress: trouble communicating with interface",
                     ::strerror(errno));
        return false;
    }
#endif
    return true;  /* LCOV_EXCL_LINE */
}

bool NetAddress::execute(ScriptOptions) const {
    output_info("installfile:" + std::to_string(this->lineno()),
                "netaddress: adding configuration for " + _iface);

    std::ofstream config("/tmp/horizon/netifrc/config_" + this->iface(),
                         std::ios_base::app);
    if(!config) {
        output_error("installfile:" + std::to_string(this->lineno()),
                     "netaddress: couldn't write network configuration for "
                     + this->iface());
        return false;
    }

    switch(this->type()) {
    case DHCP:
        config << "dhcp";
        break;
    case SLAAC:
        /* automatically handled by netifrc */
        break;
    case Static:
        config << this->address() << "/" << std::to_string(this->prefix())
               << std::endl;
        break;
    }

    if(!this->gateway().empty()) {
        std::ofstream route("/tmp/horizon/netifrc/routes_" + this->iface(),
                            std::ios_base::app);
        if(!route) {
            output_error("installfile:" + std::to_string(this->lineno()),
                         "netaddress: couldn't write route configuration for "
                         + this->iface());
            return false;
        }
        route << "default via " << this->gateway() << std::endl;
    }

    return true;
}


Key *Nameserver::parseFromData(const std::string &data, int lineno,
                               int *errors, int *) {
    char addr_buf[16];
    static const std::string valid_chars("1234567890ABCDEFabcdef:.");

    if(data.find_first_not_of(valid_chars) != std::string::npos) {
        if(errors) *errors += 0;
        output_error("installfile:" + std::to_string(lineno),
                     "nameserver: expected an IP address");
        if(data.find_first_of("[]") != std::string::npos) {
            output_info("installfile:" + std::to_string(lineno),
                        "nameserver: hint: you don't have to enclose IPv6 "
                        "addresses in [] brackets");
        }
        return nullptr;
    }

    if(data.find(':') != std::string::npos) {
        /* IPv6 */
        if(::inet_pton(AF_INET6, data.c_str(), &addr_buf) != 1) {
            if(errors) *errors += 1;
            output_error("installfile:" + std::to_string(lineno),
                         "nameserver: '" + data + "' is not a valid IPv6 "
                         "address", "hint: a ':' was found, so an IPv6 "
                         "address was expected");
            return nullptr;
        }
    } else {
        /* IPv4 */
        if(::inet_pton(AF_INET, data.c_str(), &addr_buf) != 1) {
            if(errors) *errors += 1;
            output_error("installfile:" + std::to_string(lineno),
                         "nameserver: '" + data + "' is not a valid IPv4 "
                         "address");
            return nullptr;
        }
    }

    return new Nameserver(lineno, data);
}

bool Nameserver::execute(ScriptOptions opts) const {
    if(opts.test(Simulate)) {
        std::cout << "printf 'nameserver %s\\" << "n' " << _value
                  << " >>/target/etc/resolv.conf" << std::endl;
        return true;
    }

#ifdef HAS_INSTALL_ENV
    std::ofstream resolvconf("/target/etc/resolv.conf", std::ios_base::app);
    if(!resolvconf) {
        output_error("installfile:" + std::to_string(line),
                     "nameserver: couldn't write configuration to target");
        return false;
    }
    resolvconf << "nameserver " << _value << std::endl;
#endif /* HAS_INSTALL_ENV */
    return true;  /* LCOV_EXCL_LINE */
}


Key *NetSSID::parseFromData(const std::string &data, int lineno, int *errors,
                            int *) {
    std::string iface, ssid, secstr, passphrase;
    SecurityType type;
    std::string::size_type start, pos, next;

    /* Since SSIDs can have spaces in them, we can't just naively count
     * spaces to figure a count of elements.  We have to do all the hard
     * parsing up front. :( */
    start = data.find_first_of(' ');
    if(start == std::string::npos) {
        /* ok this is just ridiculous then */
        if(errors) *errors += 1;
        output_error("installfile:" + std::to_string(lineno),
                     "netssid: at least three elements expected");
        return nullptr;
    }

    iface = data.substr(0, start);
    if(iface.length() > IFNAMSIZ) {
        if(errors) *errors += 1;
        output_error("installfile:" + std::to_string(lineno),
                     "netssid: interface name '" + iface + "' is invalid",
                     "interface names must be 16 characters or less");
        return nullptr;
    }

    if(data[start + 1] != '"') {
        if(errors) *errors += 1;
        output_error("installfile:" + std::to_string(lineno),
                     "netssid: malformed SSID", "SSIDs must be quoted");
        return nullptr;
    }

    pos = data.find_first_of('"', start + 2);
    if(pos == std::string::npos) {
        if(errors) *errors += 1;
        output_error("installfile:" + std::to_string(lineno),
                     "netssid: unterminated SSID");
        return nullptr;
    }

    ssid = data.substr(start + 2, pos - start - 2);

    if(data.length() < pos + 5) {
        if(errors) *errors += 1;
        output_error("installfile:" + std::to_string(lineno),
                     "netssid: security type expected");
        return nullptr;
    }
    start = data.find_first_of(' ', pos + 1);
    next = pos = data.find_first_of(' ', start + 1);
    /* pos may be npos if type is none.  that is fine. */
    if(pos != std::string::npos) {
        next = pos - start - 1;
    }
    secstr = data.substr(start + 1, next);
    if(secstr == "none") {
        type = SecurityType::None;
    } else if(secstr == "wep") {
        type = SecurityType::WEP;
    } else if(secstr == "wpa") {
        type = SecurityType::WPA;
    } else {
        if(errors) *errors += 1;
        output_error("installfile:" + std::to_string(lineno),
                     "netssid: unknown security type '" + secstr + "'",
                     "expected one of 'none', 'wep', or 'wpa'");
        return nullptr;
    }

    if(type != SecurityType::None) {
        if(pos == std::string::npos || data.length() < pos + 2) {
            if(errors) *errors += 1;
            output_error("installfile:" + std::to_string(lineno),
                         "netssid: expected passphrase for security type '" +
                         secstr + "'");
            return nullptr;
        }
        passphrase = data.substr(pos + 1);
    }
    return new NetSSID(lineno, iface, ssid, type, passphrase);
}

bool NetSSID::validate(ScriptOptions options) const {
    /* REQ: Runner.Validate.network.netssid.Interface */
    if(!options.test(InstallEnvironment)) {
        return true;
    }

#ifdef HAS_INSTALL_ENV
    struct iwreq request;
    int my_sock = ::socket(AF_INET, SOCK_STREAM, 0);
    if(my_sock == -1) {
        output_error("installfile:" + std::to_string(this->lineno()),
                     "netssid: can't open socket", ::strerror(errno));
        return false;
    }
    memset(&request, 0, sizeof(request));
    memcpy(&request.ifr_ifrn.ifrn_name, this->_iface.c_str(),
           this->_iface.size());
    errno = 0;
    if(ioctl(my_sock, SIOCGIWNAME, &request) == -1) {
        switch(errno)
        {
        case EOPNOTSUPP:
            output_warning("installfile:" + std::to_string(this->lineno()),
                           "netssid: specified interface is not wireless");
            return true;
        case ENODEV:
            output_warning("installfile:" + std::to_string(this->lineno()),
                           "netssid: specified interface does not exist");
            return true;
        default:
            output_error("installfile:" + std::to_string(this->lineno()),
                         "netssid: error communicating with wireless device");
            return false;
        }
    }
    ::close(my_sock);
    return true;
#else
    return false;  /* LCOV_EXCL_LINE */
#endif
}

bool NetSSID::execute(ScriptOptions) const {
    output_info("installfile:" + std::to_string(this->lineno()),
                "netssid: configuring SSID " + _ssid);

    std::ofstream conf("/tmp/horizon/wpa_supplicant.conf",
                       std::ios_base::app);
    if(!conf) {
        output_error("installfile:" + std::to_string(this->lineno()),
                     "netssid: failed to write configuration");
        return false;
    }

    conf << std::endl;
    conf << "network={" << std::endl;
    conf << "\tssid=\"" << this->ssid() << "\"" << std::endl;
    if(this->type() != SecurityType::None) {
        conf << "\tpsk=\"" << this->passphrase() << "\"" << std::endl;
    }
    conf << "\tpriority=5" << std::endl;
    conf << "}" << std::endl;

    return !conf.fail();
}
