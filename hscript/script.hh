/*
 * script.hh - Definition of the Script class
 * libhscript, the HorizonScript library for
 * Project Horizon
 *
 * Copyright (c) 2019 Adélie Linux and contributors.  All rights reserved.
 * This code is licensed under the AGPL 3.0 license, as noted in the
 * LICENSE-code file in the root directory of this repository.
 *
 * SPDX-License-Identifier: AGPL-3.0-only
 */

#include <string>
#include <vector>
#include <memory>

namespace Horizon {

/*! Defines the Script class, which represents a HorizonScript. */
class Script {
public:
    /*! Load a HorizonScript from the specified path and attempt to parse. */
    Script(std::string path);
    /*! Load a HorizonScript from the specified stream and attempt to parse. */
    Script(std::istream &stream);

    /*! Determines if the HorizonScript parsed correctly. */
    bool isParsed();
    /*! Determines if the HorizonScript is valid. */
    bool isValid();

private:
    struct ScriptPrivate;
    /*! Internal data. */
    const std::unique_ptr<ScriptPrivate> internal;
};

}
