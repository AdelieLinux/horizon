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

#include <boost/program_options.hpp>
#include <unistd.h>
#include "hscript/script.hh"
#include "util/output.hh"

bool pretty = false;

static int cli_failure(boost::program_options::options_description cli) {
        std::cout << cli << std::endl;
        return EXIT_FAILURE;
}

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

    boost::program_options::options_description cli_hidden;
    cli_hidden.add_options()
        ("installfile", "Installfile to load");
    boost::program_options::options_description cli_visible("Allowed options");
    cli_visible.add_options()
        ("install,i", "Set Installation Environment flag (DANGEROUS)")
        ("keep-going,k", "Continue parsing after errors")
        ("no-colour,n", "Do not 'prettify' output")
        ("strict,s", "Strict parsing");
    boost::program_options::options_description cli;
    cli.add(cli_visible).add(cli_hidden);
    boost::program_options::positional_options_description cli_pos;
    cli_pos.add("installfile", -1);
    boost::program_options::variables_map args;
    try {
        boost::program_options::store(
            boost::program_options::command_line_parser(argc, argv)
                .options(cli)
                .positional(cli_pos)
                .run(),
            args);
        boost::program_options::notify(args);
    } catch(const boost::program_options::error& cli_err) {
        std::cout << "An invalid option was entered." << std::endl;
        return cli_failure(cli_visible);
    }

    if (args.count("installfile")) {
        installfile = args["installfile"].as<std::string>();
    } else {
        std::cout << "You must provide an installfile." << std::endl;
        return cli_failure(cli_visible);
    }

    if (args.count("install")) {
        opts.set(ScriptOptionFlags::InstallEnvironment);
    }
    if (args.count("keep-going")) {
        opts.set(ScriptOptionFlags::KeepGoing);
    }
    if (args.count("no-colour")) {
        pretty = false;
    }
    if (args.count("strict")) {
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
