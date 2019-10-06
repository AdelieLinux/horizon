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
    bool isValid();

    /*! Determines if the Key is required to be present for the HorizonScript
     *  to be considered valid. */
    static bool isRequired();

    /*! Determines how many times this Key can be repeated.
     *  If this function returns 0, it can be repeated an unlimited number of times.
     *  If this function returns 1, it can only be present once per HorizonScript.
     */
    static int maxCount();
};

}
}
