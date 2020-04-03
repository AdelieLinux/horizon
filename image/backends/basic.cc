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

namespace Horizon {
namespace Image {

std::vector<BackendDescriptor> known_backends
__attribute__ ((init_priority(200)));

const std::vector<BackendDescriptor> BackendManager::available_backends() {
    return known_backends;
}

void BackendManager::register_backend(BackendDescriptor desc) {
    known_backends.push_back(desc);
}

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
