/*
 * script.hh - Implementation of the Script class
 * libhscript, the HorizonScript library for
 * Project Horizon
 *
 * Copyright (c) 2019 Adélie Linux and contributors.  All rights reserved.
 * This code is licensed under the AGPL 3.0 license, as noted in the
 * LICENSE-code file in the root directory of this repository.
 *
 * SPDX-License-Identifier: AGPL-3.0-only
 */

#include "script.hh"
#include "disk.hh"
#include "meta.hh"
#include "network.hh"
#include "user.hh"

namespace Horizon {

struct Script::ScriptPrivate {
    /*! Determines whether or not to enable networking. */
    std::unique_ptr<Horizon::Keys::Network> network;
    /*! The target system's hostname. */
    std::unique_ptr<Horizon::Keys::Hostname> hostname;
    /*! The packages to install to the target system. */
    std::vector<std::string> packages;
    /*! The root shadow line. */
    std::unique_ptr<Horizon::Keys::RootPassphrase> rootpw;
    /*! Target system's mountpoints. */
    std::vector< std::unique_ptr<Horizon::Keys::Mount> > mounts;
};

Script::Script() {
}

const Script *Script::load(std::string, ScriptOptions) {
    return nullptr;
}

const Script *Script::load(std::istream &, ScriptOptions) {
    return nullptr;
}

bool Script::validate() {
    return false;
}

}
