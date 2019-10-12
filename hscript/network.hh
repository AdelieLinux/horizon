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
};

class Nameserver : public Key {
};

class NetSSID : public Key {
};

}
}

#endif /* !__HSCRIPT_NETWORK_HH */
