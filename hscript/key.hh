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

#include <string>

namespace Horizon {
namespace Keys {

/*! Base Key class, used by all Keys.
 * A Getter method is not provided in this base Key class, because each Key may
 * return a different type of data.  For example, `network` may return `bool`.
 */
class Key {
public:
    /*! Create an instance of the Key. */
    static Key *create();

    /*! Set the data associated with the Key. */
    void setData(std::string data);

    /*! Determines if the data associated with the Key is valid. */
    bool validate();

    /*! Executes the action associated with this Key.
     * Will always return `false` if `validate` is `false`.
     */
    bool execute();
};

}
}
