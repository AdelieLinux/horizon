/*
 * simulator.cc - Implementation of the HorizonScript simulation utility
 * Project Horizon
 *
 * Copyright (c) 2019 Adélie Linux and contributors.  All rights reserved.
 * This code is licensed under the AGPL 3.0 license, as noted in the
 * LICENSE-code file in the root directory of this repository.
 *
 * SPDX-License-Identifier: AGPL-3.0-only
 */

#include <unistd.h>
#include "hscript/script.hh"
#include "util/output.hh"
#include "3rdparty/clipp.h"

bool pretty = false;

int main(int argc, char *argv[]) {
    const Horizon::Script *my_script;
    Horizon::ScriptOptions opts;
    int result_code = EXIT_SUCCESS;
    std::string installfile;
    using Horizon::ScriptOptionFlags;

    /* Default to pretty if we are using a TTY, unless -n specified. */
    if(isatty(1) && isatty(2)) {
        pretty = true;
    }

    opts.set(ScriptOptionFlags::Simulate);

    auto cli = (
                clipp::value("installfile", installfile),
                clipp::option("-n", "--no-colour").doc("Do not 'prettify' output")(
                    [] { pretty = false; }
                ),
                clipp::option("-s", "--strict").doc("Strict parsing")(
                    [&opts] { opts.set(ScriptOptionFlags::StrictMode); }
                )
    );
    if(!clipp::parse(argc, argv, cli)) {
        std::cout << "usage:" << std::endl;
        std::cout << clipp::usage_lines(cli, "hscript-simulate") << std::endl;
        return EXIT_FAILURE;
    }

    if(isatty(1)) {
        bold_if_pretty(std::cout);
        std::cout << "HorizonScript Simulation Utility version 0.1.0";
        reset_if_pretty(std::cout);
        std::cout << std::endl;
        std::cout << "Copyright (c) 2019 Adélie Linux and contributors.  AGPL-3.0 license." << std::endl;
        std::cout << std::endl;
    } else {
        std::cout << "#!/bin/sh" << std::endl << std::endl;
    }

    my_script = Horizon::Script::load(installfile, opts);
    if(my_script == nullptr) {
        std::cout << "Could not load the specified script." << std::endl;
        return EXIT_FAILURE;
    }

    if(!my_script->execute()) {
        output_error("installfile", "Script failed.  Stop.", "");
        result_code = EXIT_FAILURE;
    } else {
        output_info("installfile", "Script completed.", "");
    }

    delete my_script;
    return result_code;
}