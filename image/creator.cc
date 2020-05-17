/*
 * creator.cc - Implementation of the HorizonScript image creator
 * image, the image processing utilities for
 * Project Horizon
 *
 * Copyright (c) 2020 Adélie Linux and contributors.  All rights reserved.
 * This code is licensed under the AGPL 3.0 license, as noted in the
 * LICENSE-code file in the root directory of this repository.
 *
 * SPDX-License-Identifier: AGPL-3.0-only
 */

#include <boost/program_options.hpp>
#include <cstdlib>              /* EXIT_* */
#include <string>
#include "backends/basic.hh"
#include "hscript/script.hh"
#include "util/output.hh"

bool pretty = true;     /*! Controls ASCII colour output */


#define DESCR_TEXT "Usage: hscript-image [OPTION]... [INSTALLFILE]\n"\
                   "Write an operating system image configured per INSTALLFILE"
/*! Text used at the top of usage output */


/*! Entry-point for the image creation utility. */
int main(int argc, char *argv[]) {
    using namespace boost::program_options;
    using namespace Horizon::Image;

    bool needs_help{}, disable_pretty{}, version_only{};
    int exit_code = EXIT_SUCCESS;
    std::string if_path{"/etc/horizon/installfile"}, ir_dir{"/target"},
                output_path{"image.tar"}, type_code{"tar"};
    BasicBackend *backend = nullptr;
    Horizon::ScriptOptions opts;
    Horizon::Script *my_script;

    options_description ui{DESCR_TEXT};
    options_description general{"General options"};
    general.add_options()
            ("help,h", bool_switch(&needs_help), "Display this message.")
            ("no-colour,n", bool_switch(&disable_pretty), "Do not 'prettify' output.")
            ("version,v", bool_switch(&version_only), "Show program version information.")
            ;
    options_description target{"Target control options"};
    target.add_options()
            ("output,o", value<std::string>()->default_value("image.tar"), "Desired filename for the output file.")
            ("type,t", value<std::string>()->default_value("tar"), "Type of output file to generate.  Use 'list' for a list of supported types.")
            ("ir-dir,i", value<std::string>()->default_value("/target"), "Where to store intermediate files.")
            ;
    ui.add(general).add(target);

    options_description all;
    all.add(ui).add_options()("installfile", value<std::string>()->default_value(if_path), "The HorizonScript to use for configuring the image.");

    positional_options_description positional;
    positional.add("installfile", 1);

    command_line_parser parser{argc, argv};
    parser.options(all);
    parser.positional(positional);

    variables_map vm;
    try {
        auto result = parser.run();
        store(result, vm);
        notify(vm);
    } catch(const std::exception &ex) {
        std::cerr << ex.what() << std::endl;
        exit_code = EXIT_FAILURE;
        needs_help = true;
    }

    /* --help, or usage failure */
    if(needs_help) {
        std::cout << ui << std::endl;
        return exit_code;
    }

    /* -n/--no-colour, or logging to file */
    if(disable_pretty || !isatty(1)) {
        pretty = false;
    } else {
        opts.set(Horizon::Pretty);
    }

    if(!vm["type"].empty()) {
        type_code = vm["type"].as<std::string>();
    }

    if(type_code == "list") {
        std::cout << "Type codes known by this build of Image Creation:"
                  << std::endl << std::endl;
        for(const auto &candidate : BackendManager::available_backends()) {
            std::cout << std::setw(10) << std::left << candidate.type_code
                      << candidate.description << std::endl;
        }
        return EXIT_SUCCESS;
    }

    if(!vm["installfile"].empty()) {
        if_path = vm["installfile"].as<std::string>();
    }

    if(!vm["ir-dir"].empty()) {
        ir_dir = vm["ir-dir"].as<std::string>();
    }

    if(!vm["output"].empty()) {
        output_path = vm["output"].as<std::string>();
    }

    /* Announce our presence */
    bold_if_pretty(std::cout);
    std::cout << "HorizonScript Image Creation Utility version " << VERSTR
              << std::endl;
    reset_if_pretty(std::cout);
    std::cout << "Copyright (c) 2020 Adélie Linux and contributors."
              << std::endl
              << "This software is licensed to you under the terms of the "
              << std::endl << "AGPL 3.0 license, unless otherwise noted."
              << std::endl << std::endl;

    if(version_only) {
        return EXIT_SUCCESS;
    }

    /* Load the proper backend. */
    for(const auto &candidate : BackendManager::available_backends()) {
        if(candidate.type_code == type_code) {
            backend = candidate.creation_fn(ir_dir, output_path);
            break;
        }
    }
    if(backend == nullptr) {
        output_error("command-line", "unsupported backend or internal error",
                     type_code);
        return EXIT_FAILURE;
    }

    opts.set(Horizon::InstallEnvironment);
    opts.set(Horizon::ImageOnly);

    if(if_path == "-") {
        /* Unix-style stdin specification */
        my_script = Horizon::Script::load(std::cin, opts);
    } else {
        my_script = Horizon::Script::load(if_path, opts);
    }

    /* if an error occurred during parsing, bail out now */
    if(!my_script) {
        exit_code = EXIT_FAILURE;
        goto early_trouble;
    } else {
        int ret;

        my_script->setTargetDirectory(ir_dir);

        if(!my_script->execute()) {
            exit_code = EXIT_FAILURE;
            goto trouble;
        }

#define RUN_PHASE_OR_TROUBLE(_PHASE, _FRIENDLY) \
    ret = backend->_PHASE();\
    if(ret != 0) {\
        output_error("internal", "error during output " _FRIENDLY,\
                     std::to_string(ret));\
        exit_code = EXIT_FAILURE;\
        goto trouble;\
    }

        RUN_PHASE_OR_TROUBLE(prepare, "preparation");
        RUN_PHASE_OR_TROUBLE(create, "creation");
        RUN_PHASE_OR_TROUBLE(finalise, "finalisation");
    }

trouble:        /* delete the Script and exit */
    delete my_script;
early_trouble:  /* no script yet */
    delete backend;

    return exit_code;
}
