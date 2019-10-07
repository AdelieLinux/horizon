/*
 * meta.hh - Definition of the Key classes for system metadata
 * libhscript, the HorizonScript library for
 * Project Horizon
 *
 * Copyright (c) 2019 Adélie Linux and contributors.  All rights reserved.
 * This code is licensed under the AGPL 3.0 license, as noted in the
 * LICENSE-code file in the root directory of this repository.
 *
 * SPDX-License-Identifier: AGPL-3.0-only
 */

#ifndef __HSCRIPT_META_HH_
#define __HSCRIPT_META_HH_

#include "key.hh"

namespace Horizon {
namespace Keys {

class Hostname : public Key {
};

class PkgInstall : public Key {
};

class Language : public Key {
};

class Keymap : public Key {
};

class Firmware : public Key {
};

class Timezone : public Key {
};

class Repository : public Key {
};

class SigningKey : public Key {
};

}
}

#endif /* !__HSCRIPT_META_HH_ */
