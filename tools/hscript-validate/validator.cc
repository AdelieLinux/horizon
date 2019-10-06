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
#include "3rdparty/clipp.h"

int main(int argc, char *argv[]) {
    const Horizon::Script *my_script;
    Horizon::ScriptOptions opts;
    int result_code = EXIT_SUCCESS;
    std::string installfile;

    auto cli = (
                clipp::value("installfile", installfile),
                clipp::option("-k", "--keep-going").doc("Continue parsing after errors")(
                    [&opts] { opts.set(Horizon::ScriptOptionFlags::KeepGoing); }
                )
    );
    if(!clipp::parse(argc, argv, cli)) {
        std::cout << "usage:" << std::endl;
        std::cout << clipp::usage_lines(cli, "hscript-validate") << std::endl;
        return EXIT_FAILURE;
    }

    std::cout << "\033[1mHorizonScript Validation Utility version 0.1.0\033[0m" << std::endl;
    std::cout << "Copyright (c) 2019 Adélie Linux and contributors.  AGPL-3.0 license." << std::endl;
    std::cout << std::endl;

    opts.set(Horizon::ScriptOptionFlags::Pretty);

    my_script = Horizon::Script::load(installfile, opts);
    if(my_script == nullptr) {
        std::cout << "Could not load the specified script." << std::endl;
        return EXIT_FAILURE;
    }

    if(!my_script->validate()) {
        output_error("installfile", "Script failed validation step.  Stop.", "", true);
        result_code = EXIT_FAILURE;
    } else {
        output_info("installfile", "Script passed validation.", "", true);
    }

    delete my_script;
    return result_code;
}
