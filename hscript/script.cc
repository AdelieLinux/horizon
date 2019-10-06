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

namespace Horizon {

struct Script::ScriptPrivate {
    /*! Determines whether or not to enable networking. */
    bool network;
    /*! The target system's hostname. */
    std::string hostname;
    /*! The packages to install to the target system. */
    std::vector<std::string> packages;
    /*! The root shadow line. */
    std::string rootpw;
    /*! Target system's mountpoints. */
    std::vector< std::unique_ptr<Horizon::Keys::Mount> > mounts;
};

Script::Script() {
}

bool Script::load(std::string path) {
}

bool Script::load(std::istream &stream) {
}

bool Script::parse() {
}

bool Script::validate() {
}

}
