/*
 * executor.cc - Implementation of the HorizonScript executor
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

bool pretty = false;

int main(void) {
    const Horizon::Script *my_script;
    Horizon::ScriptOptions opts;
    int result_code = EXIT_SUCCESS;
    using Horizon::ScriptOptionFlags;

    /* Default to pretty if we are using a TTY, unless -n specified. */
    if(isatty(1) && isatty(2)) {
        pretty = true;  /* LCOV_EXCL_LINE */
    }

    opts.set(ScriptOptionFlags::InstallEnvironment);
    opts.set(ScriptOptionFlags::UseNetwork);
    opts.set(ScriptOptionFlags::StrictMode);

    std::cout << "HorizonScript Executor version " << VERSTR;
#ifdef NON_LIBRE_FIRMWARE
    colour_if_pretty(std::cout, "31");
    std::cout << " (supports non-free firmware)";
#endif
    reset_if_pretty(std::cout);
    std::cout << std::endl;
    std::cout << "Copyright (c) 2019 Adélie Linux and contributors.  "
                 "AGPL-3.0 license." << std::endl;
    std::cout << std::endl;

    my_script = Horizon::Script::load("/etc/horizon/installfile", opts);
    if(my_script == nullptr) {
        std::cout << "Could not load the specified script." << std::endl;
        return EXIT_FAILURE;
    }

    if(!my_script->execute()) {
        output_error("installfile", "Script failed.  Stop.", "");
        result_code = EXIT_FAILURE;
    } else {
        output_info("installfile",
                    "Adélie Linux has been successfully installed.", "");
    }

    delete my_script;
    return result_code;
}
