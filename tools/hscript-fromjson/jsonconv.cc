/*
 * jsonconv.cc - Implementation of the JSON to HorizonScript conversion utility
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
#include <fstream>
#include <sstream>
#include <string>
#include "3rdparty/json.hpp"
#include "hscript/script.hh"
#include "util/filesystem.hh"
#include "util/output.hh"

bool pretty = true;     /*! Controls ASCII colour output */

using json = nlohmann::json;


#define DESCR_TEXT "Usage: hscript-fromjson [OPTION]... [INSTALLFILE] [JSONFILE]\n"\
                   "Write an INSTALLFILE based on the description found in JSONFILE"
/*! Text used at the top of usage output */

/*! Parse a single description of a HorizonScript.
 * In JSON files that describe multiple images (i.e. YANG), this will be
 * called multiple times. */
bool parse_one_desc(json desc, std::ostream &out) {
    /* Ensure mandatory keys are present. */
#define ENSURE_KEY(node, key_name) \
    if(node.find(key_name) == node.end()) {\
        output_error("parse_one_desc", "JSON does not contain required key '"\
                                       + std::string(key_name) + "'");\
        return false;\
    }

    ENSURE_KEY(desc, "hostname");
    ENSURE_KEY(desc, "rootpw");
    ENSURE_KEY(desc, "packages");
    //ENSURE_KEY(desc, "root");

    out << "hostname " << desc["hostname"].get<std::string>() << std::endl;
    out << "rootpw " << desc["rootpw"].get<std::string>() << std::endl;
    if(desc.find("root") != desc.end()) out << "mount " << desc["root"].get<std::string>() << " /" << std::endl;

    if(desc.find("netaddresses") != desc.end()) {
        out << "network true" << std::endl;
        for(const auto &addr : desc["netaddresses"]) {
            ENSURE_KEY(addr, "interface");
            ENSURE_KEY(addr, "addr-type");
            std::string addrtype = addr["addr-type"].get<std::string>();
            out << "netaddress " << addr["interface"].get<std::string>()
                << " " << addrtype;
            if(addrtype == "static") {
                ENSURE_KEY(addr, "address");
                const auto &ipaddr = addr["address"];
                ENSURE_KEY(ipaddr, "ip-address");
                ENSURE_KEY(ipaddr, "net-prefix");
                out << " " << ipaddr["ip-address"].get<std::string>()
                    << " " << ipaddr["net-prefix"].get<std::string>();
                if(ipaddr.find("gateway") != ipaddr.end()) {
                    out << " " << ipaddr["gateway"].get<std::string>();
                }
            }
            out << std::endl;
        }
    } else {
        out << "network false" << std::endl;
    }

    std::ostringstream pkgs;
    for(const auto &pkg : desc.at("packages")) {
        pkgs << " " << pkg.get<std::string>();
    }

    out << "pkginstall" << pkgs.str() << std::endl;

#define SIMPLE_KEY(key_name) \
    if(desc.find(key_name) != desc.end()) {\
        out << key_name << " " << desc[key_name].get<std::string>()\
            << std::endl;\
    }

    SIMPLE_KEY("arch");
    SIMPLE_KEY("language");
    SIMPLE_KEY("keymap");
    SIMPLE_KEY("firmware");
    SIMPLE_KEY("timezone");

#undef SIMPLE_KEY

    if(desc.find("netconfig") != desc.end()) {
        out << "netconfigtype " << desc["netconfig"].get<std::string>()
            << std::endl;
    }

#define SIMPLE_PLURAL_KEY(key_name, hs_name) \
    if(desc.find(key_name) != desc.end()) {\
        for(const auto &element : desc[key_name]) {\
            out << hs_name << " " << element.get<std::string>() << std::endl;\
        }\
    }

    SIMPLE_PLURAL_KEY("nameservers", "nameserver");
    SIMPLE_PLURAL_KEY("repositories", "repository");
    SIMPLE_PLURAL_KEY("signingkeys", "signingkey");

#undef SIMPLE_PLURAL_KEY

    if(desc.find("services") != desc.end()) {
        for(const auto &svc : desc["services"]) {
            ENSURE_KEY(svc, "service");
            std::string service = svc["service"].get<std::string>();
            out << "svcenable " << svc;
            if(svc.find("runlevel") != svc.end()) {
                out << " " << svc["runlevel"].get<std::string>();
            }
            out << std::endl;
        }
    }

    if(desc.find("users") != desc.end()) {
        for(const auto &user : desc["users"]) {
            if(user.find("username") == user.end()) {
                output_error("input json",
                             "user account specified without username");
                continue;
            }

            const auto &name = user["username"].get<std::string>();

            out << "username " << name << std::endl;

#define USER_KEY(key_name, hscript_name) \
            if(user.find(key_name) != user.end()) {\
                out << hscript_name << " " << name << " " \
                    << user[key_name].get<std::string>() << std::endl;\
            }

            USER_KEY("alias", "useralias");
            USER_KEY("passphrase", "userpw");
            USER_KEY("groups", "usergroups");
            /* Future expansion: user icons */
        }
    }

    return true;
}


/*! Entry-point for the JSON conversion utility. */
int main(int argc, char *argv[]) {
    using namespace boost::program_options;

    bool needs_help{}, disable_pretty{}, force{}, version_only{};
    std::string if_path{"/etc/horizon/installfile"}, json_path;
    int exit_code = EXIT_SUCCESS;

    options_description ui{DESCR_TEXT "\n\nDefault HorizonScript output path: " + if_path +
                           "\nJSON will be read from standard input unless otherwise specified"};
    options_description general{"General options"};
    general.add_options()
            ("help,h", bool_switch(&needs_help), "Display this message.")
            ("version,v", bool_switch(&version_only), "Show program version information")
            ("no-colour,n", bool_switch(&disable_pretty), "Do not 'prettify' output")
            ("force,f", bool_switch(&force), "Force writing, even if HorizonScript already exists")
            ;
    ui.add(general);

    options_description all;
    all.add(ui).add_options()("installfile", value<std::string>()->default_value(if_path),
                              "The file name where the HorizonScript will be written.")
                             ("jsonfile", "The path to the JSON file.");

    positional_options_description positional;
    positional.add("installfile", 1);
    positional.add("jsonfile", 2);

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
    }

    if(!vm["installfile"].empty()) {
        if_path = vm["installfile"].as<std::string>();
    }

    if(!vm["jsonfile"].empty()) {
        json_path = vm["jsonfile"].as<std::string>();
    } else {
        json_path = "-";
    }

    if(version_only) {
        bold_if_pretty(std::cout);
        std::cout << "HorizonScript JSON Conversion Utility version " << VERSTR
                  << std::endl;
        reset_if_pretty(std::cout);
        std::cout << "Copyright (c) 2020 Adélie Linux and contributors."
                  << std::endl
                  << "This software is licensed to you under the terms of the "
                  << std::endl << "AGPL 3.0 license, unless otherwise noted."
                  << std::endl << std::endl;
        return EXIT_SUCCESS;
    }

    if(!force && fs::exists(if_path)) {
        output_error("<option>", "installfile already exists", if_path);
        return EXIT_FAILURE;
    }

    json j;
    std::string raw_json;
    char buff[8192];

    if(json_path == "-") {
        while(std::cin.getline(buff, sizeof(buff))) {
            raw_json.append(buff);
        }
    } else {
        if(!fs::exists(json_path)) {
            output_error("<option>", "JSON file does not exist", json_path);
            return EXIT_FAILURE;
        }

        auto stream = std::ifstream(json_path);
        while(stream.getline(buff, sizeof(buff))) {
            raw_json.append(buff);
        }
        if(stream.fail() && !stream.eof()) {
            output_error(json_path, "line exceeds maximum length",
                         "Maximum line length is " +
                         std::to_string(sizeof(buff)));
            return EXIT_FAILURE;
        }

        if(stream.bad() && !stream.eof()) {
            output_error(json_path, "I/O error while reading JSON");
            return EXIT_FAILURE;
        }
    }

    try {
        j = json::parse(raw_json);
    } catch(nlohmann::json::parse_error &error) {
        output_error(json_path, error.what());
        return EXIT_FAILURE;
    }

    bool res;
    if(j.find("images") != j.end()) {
        for(auto &image : j.at("images")) {
            std::string out_path = if_path + "_" + image.at("name").get<std::string>() + ".installfile";
            std::ofstream output(out_path);
            res = parse_one_desc(image, output);
            if(!res) exit_code = EXIT_FAILURE;
        }
    } else {
        if(if_path == "-") {
            res = parse_one_desc(j, std::cout);
        } else {
            std::ofstream output(if_path);
            res = parse_one_desc(j, output);
        }
        if(!res) exit_code = EXIT_FAILURE;
    }

    return exit_code;
}
