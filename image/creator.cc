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
#include "backends/backends.hh"
#include "hscript/script.hh"
#include "util/output.hh"

bool pretty = true;     /*! Controls ASCII colour output */


#define DESCR_TEXT "Usage: hscript-image [OPTION]... [INSTALLFILE]\n"\
                   "Write an operating system image configured per INSTALLFILE"
/*! Text used at the top of usage output */


/*! Entry-point for the image creation utility. */
int main(int argc, char *argv[]) {
    using namespace boost::program_options;
    bool needs_help{}, disable_pretty{};
    int exit_code = EXIT_SUCCESS;
    std::string if_path{"/etc/horizon/installfile"}, ir_dir{"/target"}, type_code{"tar"};
    Horizon::ScriptOptions opts;
    Horizon::Script *my_script;

    options_description ui{DESCR_TEXT};
    options_description general{"General options"};
    general.add_options()
            ("help,h", bool_switch(&needs_help), "Display this message.")
            ("no-colour,n", bool_switch(&disable_pretty), "Do not 'prettify' output")
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
        for(auto &backend : Horizon::Image::known_backends) {
            std::cout << std::setw(10) << std::left << backend.type_code
                      << backend.description << std::endl;
        }
        return EXIT_SUCCESS;
    }

    if(!vm["installfile"].empty()) {
        if_path = vm["installfile"].as<std::string>();
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
        return EXIT_FAILURE;
    }

    if(!vm["ir-dir"].empty()) {
        my_script->setTargetDirectory(vm["ir-dir"].as<std::string>());
    }

    my_script->execute();

    delete my_script;
    return exit_code;
}
