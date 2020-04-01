/*
 * backends.hh - Horizon Image Creation backend table
 * image, the image processing utilities for
 * Project Horizon
 *
 * Copyright (c) 2020 Adélie Linux and contributors.  All rights reserved.
 * This code is licensed under the AGPL 3.0 license, as noted in the
 * LICENSE-code file in the root directory of this repository.
 *
 * SPDX-License-Identifier: AGPL-3.0-only
 */

#pragma once

#include <functional>
#include <string>
#include <vector>

namespace Horizon {
namespace Image {

class BasicBackend;

struct BackendDescriptor {
    std::string type_code;
    std::string description;
    std::function<BasicBackend *(std::string, std::string)> creation_fn;
};

extern std::vector<BackendDescriptor> known_backends;

}
}
