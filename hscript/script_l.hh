/*
 * script_l.hh - Definition of the ScriptLocation structure
 * libhscript, the HorizonScript library for
 * Project Horizon
 *
 * Copyright (c) 2020 Adélie Linux and contributors.  All rights reserved.
 * This code is licensed under the AGPL 3.0 license, as noted in the
 * LICENSE-code file in the root directory of this repository.
 *
 * SPDX-License-Identifier: AGPL-3.0-only
 */

#ifndef SCRIPT_L_HH
#define SCRIPT_L_HH

#include <string>

namespace Horizon {

/*! Defines a location within a script. */
struct ScriptLocation {
    /*! The filename of the script. */
    std::string name;
    /*! The line number of the current location. */
    int line;
    /*! Whether this script is inherited or the original script. */
    bool inherited;

    ScriptLocation(std::string _n, int _l, bool _inherit = false) :
        name{_n}, line{_l}, inherited{_inherit} {};
};

}

#endif /* !SCRIPT_L_HH */
