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

#include <cstdlib>              /* EXIT_* */
#include <functional>
#include <map>
#include <string>
#include <vector>

#include <boost/program_options.hpp>
#include <sys/mount.h>

#include "backends/basic.hh"
#include "hscript/meta.hh"
#include "hscript/script.hh"
#include "hscript/util.hh"
#include "util/filesystem.hh"
#include "util/output.hh"

bool pretty = true;     /*! Controls ASCII colour output */


const std::string arch_xlate(const std::string &arch) {
    if(arch == "pmmx") return "i386";
    if(arch == "armv7") return "arm";
    return arch;
}


#define DESCR_TEXT "Usage: hscript-image [OPTION]... [INSTALLFILE]\n"\
                   "Write an operating system image configured per INSTALLFILE"
/*! Text used at the top of usage output */


/*! Entry-point for the image creation utility. */
int main(int argc, char *argv[]) {
    using namespace boost::program_options;
    using namespace Horizon::Image;

    bool needs_help{}, disable_pretty{}, version_only{};
    int exit_code = EXIT_SUCCESS;
    std::string if_path{"/etc/horizon/installfile"}, ir_dir{"/tmp/horizon-image"},
                output_path{"image.tar"}, type_code{"tar"};
    BasicBackend *backend = nullptr;
    std::map<std::string, std::string> backend_opts;
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
            ("ir-dir,i", value<std::string>()->default_value("/tmp/horizon-image"), "Where to store intermediate files.")
            ;
    options_description backconfig{"Backend configuration options"};
    backconfig.add_options()
            ("type,t", value<std::string>()->default_value("tar"), "Type of output file to generate.  Use 'list' for a list of supported types.")
            ("backconfig,b", value<std::vector<std::string>>(), "Set a backend configuration option.  You may specify this multiple times for multiple options.")
            ;
    ui.add(general).add(target).add(backconfig);

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

    if(fs::path(ir_dir).is_relative()) {
        ir_dir = fs::absolute(ir_dir).string();
    }

    if(!vm["output"].empty()) {
        output_path = vm["output"].as<std::string>();
    }

    if(fs::path(output_path).is_relative()) {
        output_path = fs::absolute(output_path).string();
    }

    if(!vm["backconfig"].empty()) {
        for(const auto &confpart :
                vm["backconfig"].as<std::vector<std::string>>()) {
            std::string::size_type equals = confpart.find_first_of("=");
            if(equals != std::string::npos) {
                backend_opts[confpart.substr(0, equals)] =
                        confpart.substr(equals + 1);
            } else {
                backend_opts[confpart] = "";
            }
        }
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
            backend = candidate.creation_fn(ir_dir, output_path, backend_opts);
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

#define RUN_PHASE_OR_TROUBLE(_PHASE, _FRIENDLY) \
    ret = backend->_PHASE();\
    if(ret != 0) {\
        output_error("internal", "error during output " _FRIENDLY,\
                     std::to_string(ret));\
        exit_code = EXIT_FAILURE;\
        goto trouble;\
    }

        RUN_PHASE_OR_TROUBLE(prepare, "preparation");

        /* Attempt to make images work cross-architecture.
         * This requires binfmt_misc to be configured properly.
         * It also requires qemu-user to be installed to /usr/bin on the host.
         * It may not work all the time, so we ignore errors.
         * But we try anyway, because it can work and make things easier.
         */
        const Horizon::Keys::Key *archkey = my_script->getOneValue("arch");
        std::string qpath;
        if(archkey) {
            const Horizon::Keys::Arch *arch =
                    dynamic_cast<const Horizon::Keys::Arch *>(archkey);
            qpath = "/usr/bin/qemu-" + arch_xlate(arch->value());
            error_code ec;
            if(fs::exists(qpath, ec)) {
                fs::create_directories(ir_dir + "/target/usr/bin", ec);
                if(!ec) fs::copy_file(qpath, ir_dir + "/target/" + qpath, ec);
            }
        }

        my_script->setTargetDirectory(ir_dir + "/target");

        if(!my_script->execute()) {
            exit_code = EXIT_FAILURE;
            goto trouble;
        }

        if(!qpath.empty() && fs::exists(ir_dir + "/target" + qpath)) {
            error_code ec;
            fs::remove(ir_dir + "/target" + qpath, ec);
        }

        RUN_PHASE_OR_TROUBLE(create, "creation");
        RUN_PHASE_OR_TROUBLE(finalise, "finalisation");
    }

trouble:        /* delete the Script and exit */
    /* ensure that our target mounts are unmounted */
    run_command("umount", {"-R", (ir_dir + "/target/sys")});
    umount((ir_dir + "/target/proc").c_str());
    run_command("umount", {"-R", (ir_dir + "/target/dev")});

    delete my_script;
early_trouble:  /* no script yet */
    delete backend;

    return exit_code;
}
