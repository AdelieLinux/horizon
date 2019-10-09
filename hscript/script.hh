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
#include <bitset>

namespace Horizon {

/**** Script option flags ****/

enum ScriptOptionFlags {
    /*! Don't stop after the first error. */
    KeepGoing,
    /*! Ensure network resources are available. */
    UseNetwork,
    /*! Treat warnings as errors. */
    StrictMode,
    /*! This is an Installation Environment - validate more keys. */
    InstallEnvironment,
    /*! "Pretty" output - used in interactive tooling only. */
    Pretty,
    /*! Count of flags */
    NumFlags
};


typedef std::bitset<ScriptOptionFlags::NumFlags> ScriptOptions;


/*! Defines the Script class, which represents a HorizonScript. */
class Script {
private:
    /*! Initialise the Script class. */
    Script();
    ScriptOptions opts;
public:
    /*! Load a HorizonScript from the specified path.
     * @param path      The path to load from.
     * @param options   Options to use for parsing, validation, and execution.
     * @return true if the Script could be loaded; false otherwise.
     */
    static const Script *load(const std::string path, const ScriptOptions options = 0);
    /*! Load a HorizonScript from the specified stream.
     * @param stream    The stream to load from.
     * @param options   Options to use for parsing, validation, and execution.
     * @return true if the Script could be loaded; false otherwise.
     */
    static const Script *load(std::istream &stream, const ScriptOptions options = 0);

    /*! Determines if the HorizonScript is valid. */
    bool validate() const;

private:
    struct ScriptPrivate;
    /*! Internal data. */
    ScriptPrivate *internal;
};

}

#endif /* !__HSCRIPT_SCRIPT_HH_ */
