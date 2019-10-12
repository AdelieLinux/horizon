/*
 * key.hh - Definition of the base Key classes
 * libhscript, the HorizonScript library for
 * Project Horizon
 *
 * Copyright (c) 2019 Adélie Linux and contributors.  All rights reserved.
 * This code is licensed under the AGPL 3.0 license, as noted in the
 * LICENSE-code file in the root directory of this repository.
 *
 * SPDX-License-Identifier: AGPL-3.0-only
 */

#ifndef __HSCRIPT_KEY_HH_
#define __HSCRIPT_KEY_HH_

#include <string>
#include "script.hh"

namespace Horizon {
namespace Keys {

/*! Base Key class, used by all Keys.
 * A Getter method is not provided in this base Key class, because each Key may
 * return a different type of data.  For example, `network` may return `bool`.
 */
class Key {
protected:
    /*! The line number where this Key appeared. */
    int line;
    Key(int _line) : line(_line) {}
public:
    virtual ~Key();

    /*! Create the Key object with the specified data as the entire value.
     * @param data      The value associated with the key.
     * @param lineno    The line number where the key occurs.
     * @param errors    Output variable: if not nullptr, ++ on each error.
     * @param warnings  Output variable: if not nullptr, ++ on each warning.
     * @returns nullptr if data is unparsable, otherwise a pointer to a Key.
     */
#define UNUSED __attribute__((unused))
    static Key *parseFromData(const std::string data UNUSED, int lineno UNUSED,
                              int *errors UNUSED, int *warnings UNUSED) {
        return nullptr;
    }
#undef UNUSED

    /*! Determines if the data associated with the Key is valid. */
    virtual bool validate(ScriptOptions) const = 0;

    /*! Executes the action associated with this Key.
     * @note Will always return `false` if `validate` is `false`.
     */
    virtual bool execute(ScriptOptions) const = 0;

    int lineno() const { return this->line; }
};


/*! Base Key class that parses and handles Boolean values.
 * All values passed in are lowercased.  Delimiters are not allowed.
 * Truthy values: "true" "yes" "1"
 * Falsy values: "false" "no" "0"
 * Any other values will be considered invalid.
 */
class BooleanKey : public Key {
protected:
    bool value;
    BooleanKey(int _line, bool my_value) : Key(_line), value(my_value) {}

    /*! Parse a string into a boolean.
     * @param what      The string to attempt parsing.
     * @param where     The location of the key.
     * @param key       The name of the key.
     * @param out       Output variable: will contain the value.
     * @returns true if value is parsed successfully, false otherwise.
     */
    static bool parse(const std::string what, const std::string where,
                      const std::string key, bool *out);
public:
    /*! Determines if the Key is set or not.
     * @returns true if the Key is truthy, false otherwise.
     */
    bool test() const { return this->value; }

    /*! Key will fail to init if valid is invalid. */
    bool validate(ScriptOptions) const override;
};


/*! Base Key class that parses and handles single string values. */
class StringKey : public Key {
protected:
    std::string _value;
    StringKey(int _line, std::string my_str) : Key(_line), _value(my_str) {}

public:
    /*! Retrieve the value of this key. */
    const std::string value() const { return this->_value; }

    /*! By default, key will be considered valid since it parsed.
     * This method should be overridden when further consideration is needed.
     */
    bool validate(ScriptOptions) const override;
};

}
}

#endif
