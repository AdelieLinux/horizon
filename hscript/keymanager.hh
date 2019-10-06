/*
 * keymanager.hh - Definition of the key manager
 * libhscript, the HorizonScript library for
 * Project Horizon
 *
 * Copyright (c) 2019 Adélie Linux and contributors.  All rights reserved.
 * This code is licensed under the AGPL 3.0 license, as noted in the
 * LICENSE-code file in the root directory of this repository.
 *
 * SPDX-License-Identifier: AGPL-3.0-only
 */

#include <vector>
#include <string>
#include "key.hh"

namespace Horizon {
namespace Keys {

/*! Manages the Key classes. */
class KeyManager {
private:
    /*! Create the key manager */
    KeyManager();
public:
    /*! Retrieve the global KeyManager instance. */
    static const KeyManager *getKeyManager();

    /*! Add a new Key to the key manager. */
    void addKey(std::string name, bool required, int max, Key*(*key_create_fn)(void));

    /*! Determines if a Key is recognised. */
    void hasKey(std::string name);

    /*! Create a new Key with the specified name.
     *  Returns nullptr if no Key exists.
     */
    Key *createKey(std::string name);
};

}
}
