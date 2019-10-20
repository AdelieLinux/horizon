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
        pretty = true;  /* LCOV_EXCL_LINE */
    }

    auto cli = (
                clipp::value("installfile", installfile),
                clipp::option("-i", "--install").doc("Set Installation Environment flag (DANGEROUS)")(
                    [&opts] { opts.set(ScriptOptionFlags::InstallEnvironment); }
                ),
                clipp::option("-k", "--keep-going").doc("Continue parsing after errors")(
                    [&opts] { opts.set(ScriptOptionFlags::KeepGoing); }
                ),
                clipp::option("-n", "--no-colour").doc("Do not 'prettify' output")(
                    [] { pretty = false; }
                ),
                clipp::option("-s", "--strict").doc("Strict parsing")(
                    [&opts] { opts.set(ScriptOptionFlags::StrictMode); }
                )
    );
    if(!clipp::parse(argc, argv, cli)) {
        std::cout << "usage:" << std::endl;
        std::cout << clipp::usage_lines(cli, "hscript-validate") << std::endl;
        return EXIT_FAILURE;
    }

    bold_if_pretty(std::cout);
    std::cout << "HorizonScript Validation Utility version 0.1.0";
#ifndef HAS_INSTALL_ENV
    std::cout << " (runtime environment only)";
#endif
#ifdef NON_LIBRE_FIRMWARE
    colour_if_pretty(std::cout, "31");
    std::cout << " (supports non-free firmware)";
#endif
    reset_if_pretty(std::cout);
    std::cout << std::endl;
    std::cout << "Copyright (c) 2019 Adélie Linux and contributors.  AGPL-3.0 license." << std::endl;
    std::cout << std::endl;

    my_script = Horizon::Script::load(installfile, opts);
    if(my_script == nullptr) {
        std::cout << "Could not load the specified script." << std::endl;
        return EXIT_FAILURE;
    }

    if(!my_script->validate()) {
        output_error("installfile", "Script failed validation step.  Stop.", "");
        result_code = EXIT_FAILURE;
    } else {
        output_info("installfile", "Script passed validation.", "");
    }

    delete my_script;
    return result_code;
}
