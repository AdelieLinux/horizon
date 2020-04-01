/*
 * basic.cc - Implementation of the abstract Horizon Image Creation backend
 * image, the image processing utilities for
 * Project Horizon
 *
 * Copyright (c) 2020 Adélie Linux and contributors.  All rights reserved.
 * This code is licensed under the AGPL 3.0 license, as noted in the
 * LICENSE-code file in the root directory of this repository.
 *
 * SPDX-License-Identifier: AGPL-3.0-only
 */

#include "basic.hh"
#include "backends.hh"

namespace Horizon {
namespace Image {

std::vector<BackendDescriptor> known_backends = {
   {"tar", "Create a tarball (.tar)", [](std::string, std::string){ return nullptr; } },
   {"squashfs", "Create a SquashFS image (.squashfs)", [](std::string, std::string){ return nullptr; } }
};

int BasicBackend::prepare() {
    /* The default implementation returns success immediately;
     * no preparation is required. */
    return 0;
}

int BasicBackend::finalise() {
    /* The default implementation returns success immediately;
     * no finalisation is required. */
    return 0;
}

}
}
