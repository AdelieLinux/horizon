/*
 * network.hh - Definition of the network Key classes
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
    Network(bool _value) : BooleanKey(_value) {}
public:
    static Key *parseFromData(const std::string data, int lineno, int *errors,
                              int *warnings);
    bool execute() override;
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
