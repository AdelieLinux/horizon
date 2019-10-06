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

namespace Horizon {

struct Script::ScriptPrivate {
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
