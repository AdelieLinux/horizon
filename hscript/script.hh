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

#ifndef __HSCRIPT_SCRIPT_HH_
#define __HSCRIPT_SCRIPT_HH_

#include <string>
#include <vector>
#include <memory>

namespace Horizon {

/**** Script option flags ****/

/*! Don't stop after the first error. */
#define SCRIPT_KEEP_GOING   0x0001
/*! Ensure network resources are available. */
#define SCRIPT_USE_NETWORK  0x0002
/*! Treat warnings as errors. */
#define SCRIPT_STRICT_MODE  0x0004
/*! This is an Installation Environment - validate more keys */
#define SCRIPT_INSTALL_ENV  0x0008


typedef uint32_t ScriptOptions;


/*! Defines the Script class, which represents a HorizonScript. */
class Script {
public:
    /*! Initialise the Script class. */
    Script();

    /*! Load a HorizonScript from the specified path.
     * @param path      The path to load from.
     * @param options   Options to use for parsing, validation, and execution.
     * @return true if the Script could be loaded; false otherwise.
     */
    static const Script *load(std::string path, ScriptOptions options = 0);
    /*! Load a HorizonScript from the specified stream.
     * @param stream    The stream to load from.
     * @param options   Options to use for parsing, validation, and execution.
     * @return true if the Script could be loaded; false otherwise.
     */
    static const Script *load(std::istream &stream, ScriptOptions options = 0);

    /*! Determines if the HorizonScript is valid. */
    bool validate();

private:
    struct ScriptPrivate;
    /*! Internal data. */
    const std::unique_ptr<ScriptPrivate> internal;
};

}

#endif /* !__HSCRIPT_SCRIPT_HH_ */
