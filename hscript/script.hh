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

#include <hscript/script_l.hh>

namespace Horizon {

namespace Keys {

class Key;

}

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
    /*! Just print commands that would be run, for testing/debug */
    Simulate,
    /*! Installing to an image; don't mount anything */
    ImageOnly,
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
    /*! Free resources associated with the Script. */
    ~Script();

    /*! Load a HorizonScript from the specified path.
     * @param path      The path to load from.
     * @param options   Options to use for parsing, validation, and execution.
     * @return true if the Script could be loaded; false otherwise.
     */
    static Script *load(const std::string &path,
                        const ScriptOptions &options = 0);
    /*! Load a HorizonScript from the specified stream.
     * @param stream    The stream to load from.
     * @param options   Options to use for parsing, validation, and execution.
     * @param name      The name of the stream to use in diagnostic messages.
     * @return true if the Script could be loaded; false otherwise.
     */
    static Script *load(std::istream &stream,
                        const ScriptOptions &options = 0,
                        const std::string &name = "installfile");

    /*! Determines if the HorizonScript is valid. */
    bool validate() const;

    /*! Executes the HorizonScript. */
    bool execute() const;

    /*! Retrieve the current target directory. */
    const std::string targetDirectory() const;

    /*! Set the current target directory. */
    void setTargetDirectory(const std::string &dir);

    /*! Retrieve the value of a specified key in this HorizonScript.
     * @param name      The name of the key to retrieve.
     * @return The key object, if one exists.  nullptr if the key has not been
     *         specified.
     */
    const Keys::Key *getOneValue(std::string name) const;

    /*! Retrieve all values for a specified key in this HorizonScript.
     * @param name      The name of the key to retrieve.
     * @return A std::vector of the key objects.  The vector may be empty if
     *         no values exist for the specified key.
     */
    const std::vector<Keys::Key *> getValues(std::string name) const;

    /*! Retrieve the options set for this HorizonScript object. */
    ScriptOptions options() const;
private:
    struct ScriptPrivate;
    /*! Internal data. */
    ScriptPrivate *internal;
};

}

#endif /* !__HSCRIPT_SCRIPT_HH_ */
