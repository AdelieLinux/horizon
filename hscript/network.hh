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

namespace Horizon {
namespace Keys {

class Network : public Key {
public:
    /*! Determine if networking is enabled. */
    bool enabled();
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
