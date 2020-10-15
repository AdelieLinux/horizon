/*
 * network.cc - Implementation of the Key classes for network configuration
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
#include <arpa/inet.h>          /* inet_pton */
#include <cstring>              /* memcpy */
#include <fstream>              /* ofstream for Net write */
#include <set>                  /* for PPPoE valid param keys */
#ifdef HAS_INSTALL_ENV
#   include <linux/wireless.h>     /* struct iwreq */
#   include <string.h>             /* strerror */
#   include <sys/ioctl.h>          /* ioctl, ioctl numbers */
#   include <unistd.h>             /* close */
#   include "util/filesystem.hh"   /* fs for pppoe eni */
#else
/*! The size of Linux interface names. */
#   define IFNAMSIZ 16
#endif
#include "network.hh"
#include "util/net.hh"
#include "util/output.hh"

using namespace Horizon::Keys;


Key *Network::parseFromData(const std::string &data, const ScriptLocation &pos,
                            int *errors, int *, const Script *script) {
    bool value;
    if(!BooleanKey::parse(data, pos, "network", &value)) {
        if(errors) *errors += 1;
        return nullptr;
    }
    return new Network(script, pos, value);
}

bool Network::execute() const {
    /* The network key, by itself, does nothing. */
    return true;
}


Key *NetConfigType::parseFromData(const std::string &data,
                                  const ScriptLocation &pos,
                                  int *errors, int *, const Script *script) {
    std::string type = data;
    ConfigSystem system;

    std::transform(type.cbegin(), type.cend(), type.begin(), ::tolower);

    if(type == "netifrc") {
        system = Netifrc;
    } else if(type == "eni") {
        system = ENI;
    } else {
        if(errors) *errors += 1;
        output_error(pos, "netconfigtype: invalid or missing config type",
                     "one of 'netifrc', 'eni' required");
        return nullptr;
    }

    return new NetConfigType(script, pos, system);
}

/* LCOV_EXCL_START */
bool NetConfigType::validate() const {
    /* Validation takes place during parsing. */
    return true;
}

bool NetConfigType::execute() const {
    /* This key doesn't perform any action by itself. */
    return true;
}
/* LCOV_EXCL_STOP */


Key *NetAddress::parseFromData(const std::string &data,
                               const ScriptLocation &pos, int *errors, int *,
                               const Script *script) {
    long elements = std::count(data.cbegin(), data.cend(), ' ') + 1;
    std::string::size_type type_pos, addr_pos, prefix_pos, gw_pos, next_end;
    std::string iface, type, addr, prefix, gw;
    int real_prefix;
    char addr_buf[16];

    if(elements < 2) {
        if(errors) *errors += 1;
        output_error(pos, "netaddress: missing address type",
                     "one of 'dhcp', 'slaac', 'static' required");
        return nullptr;
    }

    type_pos = data.find_first_of(' ');
    iface = data.substr(0, type_pos);
    if(iface.length() > IFNAMSIZ) {
        if(errors) *errors += 1;
        output_error(pos, "netaddress: invalid interface name '" + iface + "'",
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
            output_error(pos, "netaddress: address type 'dhcp' does not "
                         "accept further elements");
            return nullptr;
        }
        return new NetAddress(script, pos, iface, AddressType::DHCP, "", 0,
                              "");
    } else if(!type.compare("slaac")) {
        if(elements > 2) {
            if(errors) *errors += 1;
            output_error(pos, "netaddress: address type 'slaac' does not "
                         "accept further elements");
            return nullptr;
        }
        return new NetAddress(script, pos, iface, AddressType::SLAAC, "", 0,
                              "");
    } else if(type.compare("static")) {
        if(errors) *errors += 1;
        output_error(pos, "netaddress: invalid address type '" + type + "'",
                     "one of 'dhcp', 'slaac', 'static' required");
        return nullptr;
    }

    /* static address */
    if(elements < 4) {
        if(errors) *errors += 1;
        output_error(pos, "netaddress: address type 'static' requires at "
                     "least an IP address and prefix length");
        return nullptr;
    }

    if(elements > 5) {
        if(errors) *errors += 1;
        output_error(pos, "netaddress: too many elements for static address");
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
            output_error(pos, "netaddress: '" + addr + "' is not a valid IPv6 address",
                         "hint: a ':' was found, indicating this address is IPv6");
            return nullptr;
        }

        try {
            real_prefix = std::stoi(prefix);
        } catch(const std::exception &) {
            if(errors) *errors += 1;
            output_error(pos, "netaddress: prefix length is not a number",
                         "prefix must be a decimal value between 1 and 128");
            return nullptr;
        }

        if(real_prefix < 1 || real_prefix > 128) {
            if(errors) *errors += 1;
            output_error(pos, "netaddress: invalid IPv6 prefix length: " + prefix,
                         "prefix must be a decimal value between 1 and 128");
            return nullptr;
        }

        if(gw.size() > 0 &&
           ::inet_pton(AF_INET6, gw.c_str(), &addr_buf) != 1) {
            if(errors) *errors += 1;
            output_error(pos, "netaddress: '" + gw +
                         "' is not a valid IPv6 gateway",
                         "an IPv6 address must have an IPv6 gateway");
            return nullptr;
        }

        return new NetAddress(script, pos, iface, AddressType::Static, addr,
                              static_cast<uint8_t>(real_prefix), gw);
    } else if(addr.find('.') != std::string::npos) {
        /* IPv4 */
        if(::inet_pton(AF_INET, addr.c_str(), &addr_buf) != 1) {
            if(errors) *errors += 1;
            output_error(pos, "netaddress: invalid IPv4 address", addr);
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
                output_error(pos, "netaddress: can't parse prefix length/mask",
                             "a network mask or prefix length is required");
                return nullptr;
            }
        }

        if(real_prefix < 1 || real_prefix > 32) {
            if(errors) *errors += 1;
            output_error(pos, "netaddress: invalid IPv4 prefix length: " +
                         prefix, "prefix must be between 1 and 32");
            return nullptr;
        }

        if(gw.size() > 0 &&
           ::inet_pton(AF_INET, gw.c_str(), &addr_buf) != 1) {
            if(errors) *errors += 1;
            output_error(pos, "netaddress: '" + gw +
                         "' is not a valid IPv4 gateway",
                         "an IPv4 address must have an IPv4 gateway");
            return nullptr;
        }

        return new NetAddress(script, pos, iface, AddressType::Static, addr,
                              static_cast<uint8_t>(real_prefix), gw);
    } else {
        /* IPvBad */
        if(errors) *errors += 1;
        output_error(pos, "netaddress: invalid address of unknown type",
                     "an IPv4 or IPv6 address is required");
        return nullptr;
    }
}

bool NetAddress::validate() const {
    if(!script->options().test(InstallEnvironment)) {
        return true;
    }

#ifdef HAS_INSTALL_ENV
    /* Retrieving the index is always valid, and is not even privileged. */
    struct ifreq request;
    int my_sock = ::socket(AF_INET, SOCK_STREAM, 0);
    if(my_sock == -1) {
        output_error(pos, "netaddress: can't open socket", ::strerror(errno));
        return false;
    }
    memset(&request, 0, sizeof(request));
    memcpy(&request.ifr_name, _iface.c_str(), _iface.size());
    errno = 0;
    if(ioctl(my_sock, SIOCGIFFLAGS, &request) == -1) {
        if(errno == ENODEV) {
            output_warning(pos, "netaddress: interface does not exist", _iface);
            return true;
        }
        output_error(pos, "netaddress: trouble communicating with interface",
                     ::strerror(errno));
        return false;
    }
#endif
    return true;  /* LCOV_EXCL_LINE */
}

bool execute_address_netifrc(const NetAddress *addr) {
    std::ofstream config("/tmp/horizon/netifrc/config_" + addr->iface(),
                         std::ios_base::app);
    if(!config) {
        output_error(addr->where(), "netaddress: couldn't write network "
                     "configuration for " + addr->iface());
        return false;
    }

    switch(addr->type()) {
    case NetAddress::DHCP:
        config << "dhcp";
        break;
    case NetAddress::SLAAC:
        /* automatically handled by netifrc */
        break;
    case NetAddress::Static:
        config << addr->address() << "/" << std::to_string(addr->prefix())
               << std::endl;
        break;
    }

    if(!addr->gateway().empty()) {
        std::ofstream route("/tmp/horizon/netifrc/routes_" + addr->iface(),
                            std::ios_base::app);
        if(!route) {
            output_error(addr->where(), "netaddress: couldn't write route "
                         "configuration for " + addr->iface());
            return false;
        }
        route << "default via " << addr->gateway() << std::endl;
    }

    return true;
}

bool execute_address_eni(const NetAddress *addr) {
    std::ofstream config("/tmp/horizon/eni/" + addr->iface(),
                         std::ios_base::app);
    if(!config) {
        output_error(addr->where(), "netaddress: couldn't write network "
                     "configuration for " + addr->iface());
        return false;
    }

    switch(addr->type()) {
    case NetAddress::DHCP:
        config << "iface " << addr->iface() << " inet dhcp" << std::endl;
        break;
    case NetAddress::SLAAC:
        config << "iface " << addr->iface() << " inet6 manual" << std::endl
               << "\tpre-up echo 1 > /proc/sys/net/ipv6/conf/" << addr->iface()
               << "/accept_ra" << std::endl;
        break;
    case NetAddress::Static:
        config << "iface " << addr->iface() << " ";

        if(addr->address().find(':') != std::string::npos) {
            /* Disable SLAAC when using static IPv6 addressing. */
            config << "inet6 static" << std::endl
                   << "\tpre-up echo 0 > /proc/sys/net/ipv6/conf/"
                   << addr->iface() << "/accept_ra" << std::endl;;
        } else {
            config << "inet static" << std::endl;
        }

        config << "\taddress " << addr->address() << std::endl
               << "\tnetmask " << std::to_string(addr->prefix()) << std::endl;

        if(!addr->gateway().empty()) {
            config << "\tgateway " << addr->gateway() << std::endl;
        }
    }

    return true;
}

NetConfigType::ConfigSystem current_system(const Horizon::Script *script) {
    NetConfigType::ConfigSystem system = NetConfigType::Netifrc;

    const Key *key = script->getOneValue("netconfigtype");
    if(key != nullptr) {
        const NetConfigType *nct = static_cast<const NetConfigType *>(key);
        system = nct->type();
    }

    return system;
}

bool NetAddress::execute() const {
    output_info(pos, "netaddress: adding configuration for " + _iface);

    switch(current_system(script)) {
    case NetConfigType::Netifrc:
    default:
        return execute_address_netifrc(this);
    case NetConfigType::ENI:
        return execute_address_eni(this);
    }
}


Key *PPPoE::parseFromData(const std::string &data, const ScriptLocation &pos,
                          int *errors, int *, const Script *script) {
    std::string::size_type spos, next;
    std::map<std::string, std::string> params;
    std::string iface;

    spos = data.find_first_of(' ');
    iface = data.substr(0, spos);
    if(iface.length() > IFNAMSIZ) {
        if(errors) *errors += 1;
        output_error(pos, "pppoe: invalid interface name '" + iface + "'",
                     "interface names must be 16 characters or less");
        return nullptr;
    }

    while(spos != std::string::npos) {
        std::string key, val;
        std::string::size_type equals;

        spos++;

        next = data.find_first_of(' ', spos);
        equals = data.find_first_of('=', spos);
        if(equals != std::string::npos && equals < next) {
            key = data.substr(spos, equals - spos);
            if(next == std::string::npos) {
                val = data.substr(equals + 1);
            } else {
                val = data.substr(equals + 1, next - equals - 1);
            }
        } else {
            if(next == std::string::npos) {
                key = data.substr(spos);
            } else {
                key = data.substr(spos, next - spos);
            }
        }

        params[key] = val;

        spos = next;
    }

    return new PPPoE(script, pos, iface, params);
}

bool PPPoE::validate() const {
    bool valid = true;
    const std::set<std::string> valid_keys = {"mtu", "username", "password",
                                              "lcp-echo-interval",
                                              "lcp-echo-failure"};

    for(const auto &elem : this->params()) {
        if(valid_keys.find(elem.first) == valid_keys.end()) {
            output_error(this->pos, "pppoe: invalid parameter", elem.first);
            valid = false;
        }
    }

    return valid;
}

static int ppp_link_count = 0;

bool execute_pppoe_netifrc(const PPPoE &link) {
    const auto &params = link.params();
    const std::string &linkiface{"ppp" + std::to_string(ppp_link_count)};

#define ENSURE_FILE(file) \
    if(!file) {\
        output_error(link.where(), "pppoe: couldn't write network "\
                     "configuration for " + linkiface);\
        return false;\
    }

    std::ofstream ethconfig("/tmp/horizon/netifrc/config_" + link.iface(),
                            std::ios_base::trunc);
    ENSURE_FILE(ethconfig);
    ethconfig << "null";

    std::ofstream rcconfig("/tmp/horizon/netifrc/rc_net_" + linkiface + "_need");
    ENSURE_FILE(rcconfig);
    rcconfig << link.iface();

    std::ofstream config("/tmp/horizon/netifrc/config_" + linkiface);
    ENSURE_FILE(config);
    config << "ppp";

    std::ofstream linkconfig("/tmp/horizon/netifrc/link_" + linkiface);
    ENSURE_FILE(linkconfig);
    linkconfig << link.iface();

    std::ofstream plugconfig("/tmp/horizon/netifrc/plugins_" + linkiface);
    ENSURE_FILE(plugconfig);
    plugconfig << "pppoe";

    if(params.find("username") != params.end()) {
        std::ofstream userconfig("/tmp/horizon/netifrc/username_" + linkiface);
        ENSURE_FILE(userconfig);
        userconfig << params.at("username");
    }

    if(params.find("password") != params.end()) {
        std::ofstream pwconfig("/tmp/horizon/netifrc/password_" + linkiface);
        ENSURE_FILE(pwconfig);
        pwconfig << params.at("password");
    }

    std::ofstream pppconfig("/tmp/horizon/netifrc/pppd_" + linkiface);
    ENSURE_FILE(pppconfig);
    pppconfig << "noauth" << std::endl
              << "defaultroute" << std::endl;

    if(params.find("lcp-echo-interval") != params.end()) {
        pppconfig << "lcp-echo-interval " << params.at("lcp-echo-interval")
                  << std::endl;
    }

    if(params.find("lcp-echo-failure") != params.end()) {
        pppconfig << "lcp-echo-failure " << params.at("lcp-echo-failure")
                  << std::endl;
    }

    if(params.find("mtu") != params.end()) {
        pppconfig << "mtu " << params.at("mtu") << std::endl;
    }

    ppp_link_count++;
    return true;
}

bool execute_pppoe_eni(const PPPoE &link, const Horizon::Script *script) {
#ifndef HAS_INSTALL_ENV
    output_error(link.where(), "pppoe: ENI cannot be simulated");
    return false;
#else
    const auto &params = link.params();
    const std::string &pppdir{script->targetDirectory() + "/etc/ppp"};
    const std::string &linkiface{"ppp" + std::to_string(ppp_link_count)};
    error_code ec;

    fs::create_directories(pppdir + "/peers", ec);
    if(ec && ec.value() != EEXIST) {
        output_error(link.where(), "pppoe: cannot create PPP directory",
                     ec.message());
        return false;
    }

    std::ofstream config("/tmp/horizon/eni/" + link.iface(),
                         std::ios_base::trunc);
    if(!config) {
        output_error(link.where(), "pppoe: couldn't write network "
                     "configuration for " + link.iface());
        return false;
    }
    config << "iface " << linkiface << " inet ppp" << std::endl
           << "pre-up /sbin/ifconfig " << link.iface() << " up" << std::endl
           << "provider " << linkiface;

    std::ofstream pppconf(pppdir + "/peers/" + linkiface);
    if(!pppconf) {
        output_error(link.where(), "pppoe: couldn't write peer information");
        return false;
    }

    pppconf << "plugin rp-pppoe.so" << std::endl
            << link.iface() << std::endl
            << "defaultroute" << std::endl
            << "noauth" << std::endl
            << "+ipv6" << std::endl;

    for(const auto &cpair : params) {
        if(cpair.first == "password") continue;

        std::string key{cpair.first};
        /* rewrite our keys -> pppd keys */
        if(key == "username") key = "user";

        pppconf << key;
        if(cpair.second.size() > 0) pppconf << " " << cpair.second;
        pppconf << std::endl;
    }

    if(params.find("password") != params.end()) {
        if(params.find("username") == params.end()) {
            output_error(link.where(), "pppoe: password without username "
                         "not supported in ENI mode");
            return false;
        }
        std::ofstream secrets(pppdir + "/chap-secrets");
        secrets << params.at("username") << "\t*\t" << params.at("password")
                << std::endl;
    }

    ppp_link_count++;
    return true;
#endif
}

bool PPPoE::execute() const {
    output_info(pos, "pppoe: adding configuration for " + _iface);

    switch(current_system(script)) {
    case NetConfigType::Netifrc:
    default:
        return execute_pppoe_netifrc(*this);
    case NetConfigType::ENI:
        return execute_pppoe_eni(*this, script);
    }
}


Key *Nameserver::parseFromData(const std::string &data,
                               const ScriptLocation &pos, int *errors, int *,
                               const Script *script) {
    char addr_buf[16];
    static const std::string valid_chars("1234567890ABCDEFabcdef:.");

    if(data.find_first_not_of(valid_chars) != std::string::npos) {
        if(errors) *errors += 0;
        output_error(pos, "nameserver: expected an IP address");
        if(data.find_first_of("[]") != std::string::npos) {
            output_info(pos, "nameserver: hint: you don't have to enclose IPv6 "
                        "addresses in [] brackets");
        }
        return nullptr;
    }

    if(data.find(':') != std::string::npos) {
        /* IPv6 */
        if(::inet_pton(AF_INET6, data.c_str(), &addr_buf) != 1) {
            if(errors) *errors += 1;
            output_error(pos, "nameserver: '" + data + "' is not a valid IPv6 "
                         "address", "hint: a ':' was found, so an IPv6 "
                         "address was expected");
            return nullptr;
        }
    } else {
        /* IPv4 */
        if(::inet_pton(AF_INET, data.c_str(), &addr_buf) != 1) {
            if(errors) *errors += 1;
            output_error(pos, "nameserver: '" + data + "' is not a valid IPv4 "
                         "address");
            return nullptr;
        }
    }

    return new Nameserver(script, pos, data);
}

bool Nameserver::execute() const {
    if(script->options().test(Simulate)) {
        std::cout << "printf 'nameserver %s\\" << "n' " << _value
                  << " >>" << script->targetDirectory() << "/etc/resolv.conf"
                  << std::endl;
        return true;
    }

#ifdef HAS_INSTALL_ENV
    std::ofstream resolvconf(script->targetDirectory() + "/etc/resolv.conf",
                             std::ios_base::app);
    if(!resolvconf) {
        output_error(pos, "nameserver: couldn't write configuration to target");
        return false;
    }
    resolvconf << "nameserver " << _value << std::endl;
#endif /* HAS_INSTALL_ENV */
    return true;  /* LCOV_EXCL_LINE */
}


Key *NetSSID::parseFromData(const std::string &data, const ScriptLocation &p,
                            int *errors, int *, const Script *script) {
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
        output_error(p, "netssid: at least three elements expected");
        return nullptr;
    }

    iface = data.substr(0, start);
    if(iface.length() > IFNAMSIZ) {
        if(errors) *errors += 1;
        output_error(p, "netssid: interface name '" + iface + "' is invalid",
                     "interface names must be 16 characters or less");
        return nullptr;
    }

    if(data[start + 1] != '"') {
        if(errors) *errors += 1;
        output_error(p, "netssid: malformed SSID", "SSIDs must be quoted");
        return nullptr;
    }

    pos = data.find_first_of('"', start + 2);
    if(pos == std::string::npos) {
        if(errors) *errors += 1;
        output_error(p, "netssid: unterminated SSID");
        return nullptr;
    }

    ssid = data.substr(start + 2, pos - start - 2);

    if(data.length() < pos + 5) {
        if(errors) *errors += 1;
        output_error(p, "netssid: security type expected");
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
        output_error(p, "netssid: unknown security type '" + secstr + "'",
                     "expected one of 'none', 'wep', or 'wpa'");
        return nullptr;
    }

    if(type != SecurityType::None) {
        if(pos == std::string::npos || data.length() < pos + 2) {
            if(errors) *errors += 1;
            output_error(p, "netssid: expected passphrase for security type '" +
                         secstr + "'");
            return nullptr;
        }
        passphrase = data.substr(pos + 1);
    }
    return new NetSSID(script, p, iface, ssid, type, passphrase);
}

bool NetSSID::validate() const {
    /* REQ: Runner.Validate.network.netssid.Interface */
    if(!script->options().test(InstallEnvironment)) {
        return true;
    }

#ifdef HAS_INSTALL_ENV
    struct iwreq request;
    int my_sock = ::socket(AF_INET, SOCK_STREAM, 0);
    if(my_sock == -1) {
        output_error(pos, "netssid: can't open socket", ::strerror(errno));
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
            output_warning(pos, "netssid: specified interface is not wireless");
            return true;
        case ENODEV:
            output_warning(pos, "netssid: specified interface does not exist");
            return true;
        default:
            output_error(pos, "netssid: error communicating with device");
            return false;
        }
    }
    ::close(my_sock);
    return true;
#else
    return false;  /* LCOV_EXCL_LINE */
#endif
}

bool NetSSID::execute() const {
    output_info(pos, "netssid: configuring SSID " + _ssid);

    std::ofstream conf("/tmp/horizon/wpa_supplicant.conf",
                       std::ios_base::app);
    if(!conf) {
        output_error(pos, "netssid: failed to write configuration");
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
