/*
 * key.hh - Definition of the base Key class
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

namespace Horizon {
namespace Keys {

/*! Base Key class, used by all Keys.
 * A Getter method is not provided in this base Key class, because each Key may
 * return a different type of data.  For example, `network` may return `bool`.
 */
class Key {
public:
    virtual ~Key();

    /*! Create the Key object with the specified data as the entire value.
     * @param data      The value associated with the key.
     * @param lineno    The line number where the key occurs.
     * @param errors    Output variable: if not nullptr, ++ on each error.
     * @param warnings  Output variable: if not nullptr, ++ on each warning.
     * @returns nullptr if data is unparsable, otherwise a pointer to a Key.
     */
    static Key *parseFromData(std::string data, int lineno, int *errors,
                              int *warnings) { return nullptr; }

    /*! Determines if the data associated with the Key is valid. */
    virtual bool validate() = 0;

    /*! Executes the action associated with this Key.
     * @note Will always return `false` if `validate` is `false`.
     */
    virtual bool execute() = 0;
};

}
}

#endif
