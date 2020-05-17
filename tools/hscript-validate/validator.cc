/*
 * validator.cc - Implementation of the HorizonScript validation utility
 * Project Horizon
 *
 * Copyright (c) 2019-2020 Adélie Linux and contributors.  All rights reserved.
 * This code is licensed under the AGPL 3.0 license, as noted in the
 * LICENSE-code file in the root directory of this repository.
 *
 * SPDX-License-Identifier: AGPL-3.0-only
 */

#include <boost/program_options.hpp>
#include <unistd.h>
#include "hscript/script.hh"
#include "util/output.hh"


bool pretty = false;


int main(int argc, char *argv[]) {
    const Horizon::Script *my_script;
    Horizon::ScriptOptions opts;
    int result_code = EXIT_SUCCESS;
    std::string installfile;
    bool install{}, keep_going{}, strict{}, needs_help{}, version_only{},
         dont_pretty{};
    using Horizon::ScriptOptionFlags;
    using namespace boost::program_options;

    /* Default to pretty if we are using a TTY, unless -n specified. */
    if(isatty(1) && isatty(2)) {
        pretty = true;  /* LCOV_EXCL_LINE */
    }

    options_description cli_hidden;
    cli_hidden.add_options()
        ("installfile", "Installfile to load");
    options_description cli_visible("Allowed options");
    cli_visible.add_options()
        ("help,h", bool_switch(&needs_help), "Display this message.")
        ("version,v", bool_switch(&version_only), "Show program version information.")
        ("install,i", bool_switch(&install), "Set Installation Environment flag. (DANGEROUS!)")
        ("keep-going,k", bool_switch(&keep_going), "Continue parsing after errors.")
        ("no-colour,n", bool_switch(&dont_pretty), "Do not 'prettify' output.")
        ("strict,s", bool_switch(&strict), "Use strict parsing mode (enable more warnings/errors).");
    options_description cli;
    cli.add(cli_visible).add(cli_hidden);
    positional_options_description cli_pos;
    cli_pos.add("installfile", -1);
    variables_map args;
    try {
        store(
            command_line_parser(argc, argv)
                .options(cli)
                .positional(cli_pos)
                .run(),
            args);
        notify(args);
    } catch(const boost::program_options::error& cli_err) {
        std::cout << "An invalid option was entered." << std::endl;
        needs_help = true;
        result_code = EXIT_FAILURE;
    }

    if(needs_help) {
        std::cout << cli_visible << std::endl;
        return result_code;
    }

    if(install) {
        opts.set(ScriptOptionFlags::InstallEnvironment);
    }
    if(keep_going) {
        opts.set(ScriptOptionFlags::KeepGoing);
    }
    if(dont_pretty) {
        pretty = false;
    }
    if(strict) {
        opts.set(ScriptOptionFlags::StrictMode);
    }

    bold_if_pretty(std::cout);
    std::cout << "HorizonScript Validation Utility version " << VERSTR;
#ifndef HAS_INSTALL_ENV
    std::cout << " (runtime environment only)";
#endif
#ifdef NON_LIBRE_FIRMWARE
    colour_if_pretty(std::cout, "31");
    std::cout << " (supports non-free firmware)";
#endif
    reset_if_pretty(std::cout);
    std::cout << std::endl
              << "Copyright (c) 2019-2020 Adélie Linux and contributors."
              << std::endl
              << "This software is licensed to you under the terms of the "
              << std::endl << "AGPL 3.0 license, unless otherwise noted."
              << std::endl << std::endl;

    if(version_only) {
        return EXIT_SUCCESS;
    }

    if(args.count("installfile")) {
        installfile = args["installfile"].as<std::string>();
    } else {
        output_error("<stdin>", "You must specify an installfile");
        return EXIT_FAILURE;
    }

    my_script = Horizon::Script::load(installfile, opts);
    if(my_script == nullptr) {
        output_error(installfile, "Could not load the specified script");
        return EXIT_FAILURE;
    }

    if(!my_script->validate()) {
        output_error("internal", "Script failed validation.  Stop.");
        result_code = EXIT_FAILURE;
    } else {
        output_info("internal", "Script passed validation.");
    }

    delete my_script;
    return result_code;
}
