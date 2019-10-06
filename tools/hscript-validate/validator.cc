/*
 * validator.cc - Implementation of the HorizonScript validation utility
 * Project Horizon
 *
 * Copyright (c) 2019 Adélie Linux and contributors.  All rights reserved.
 * This code is licensed under the AGPL 3.0 license, as noted in the
 * LICENSE-code file in the root directory of this repository.
 *
 * SPDX-License-Identifier: AGPL-3.0-only
 */

#include "hscript/script.hh"
#include "util/output.hh"

int main(int argc, char *argv[]) {
    const Horizon::Script *my_script;
    Horizon::ScriptOptions opts;

    if(argc < 2) {
        output_error("hscript-validate", "No installfile specified", "", true);
        std::cerr << "Run `" << argv[0] << " --help` for usage information." << std::endl;
        return EXIT_FAILURE;
    }

    std::cout << "\033[1mHorizonScript Validation Utility version 0.1.0\033[0m" << std::endl;
    std::cout << "Copyright (c) 2019 Adélie Linux and contributors.  AGPL-3.0 license." << std::endl;
    std::cout << std::endl;

    opts.set(Horizon::ScriptOptionFlags::Pretty);

    my_script = Horizon::Script::load(argv[1], opts);
    if(my_script == nullptr) {
        std::cout << "Could not load the specified script." << std::endl;
        return EXIT_FAILURE;
    }
    delete my_script;

    return EXIT_SUCCESS;
}
