/*
 * script.cc - Implementation of the Script class
 * libhscript, the HorizonScript library for
 * Project Horizon
 *
 * Copyright (c) 2019 Adélie Linux and contributors.  All rights reserved.
 * This code is licensed under the AGPL 3.0 license, as noted in the
 * LICENSE-code file in the root directory of this repository.
 *
 * SPDX-License-Identifier: AGPL-3.0-only
 */

#include <algorithm>
#include <fstream>
#include <iostream>
#include <map>
#include <set>
#include <sstream>

#include "script.hh"
#include "disk.hh"
#include "meta.hh"
#include "network.hh"
#include "user.hh"

#include "util/output.hh"

#define LINE_MAX 512


typedef Horizon::Keys::Key *(*key_parse_fn)(std::string, int, int*, int*);

const std::map<std::string, key_parse_fn> valid_keys = {
    {"network", &Horizon::Keys::Network::parseFromData},
    {"hostname", &Horizon::Keys::Hostname::parseFromData},
    {"pkginstall", &Horizon::Keys::PkgInstall::parseFromData},
    {"rootpw", &Horizon::Keys::RootPassphrase::parseFromData},

    {"language", &Horizon::Keys::Language::parseFromData},
    {"keymap", &Horizon::Keys::Keymap::parseFromData},
    {"firmware", &Horizon::Keys::Firmware::parseFromData},
    {"timezone", &Horizon::Keys::Timezone::parseFromData},
    {"repository", &Horizon::Keys::Repository::parseFromData},
    {"signingkey", &Horizon::Keys::SigningKey::parseFromData},

    {"netaddress", &Horizon::Keys::NetAddress::parseFromData},
    {"nameserver", &Horizon::Keys::Nameserver::parseFromData},
    {"netssid", &Horizon::Keys::NetSSID::parseFromData},

    {"username", &Horizon::Keys::Username::parseFromData},
    {"useralias", &Horizon::Keys::UserAlias::parseFromData},
    {"userpw", &Horizon::Keys::UserPassphrase::parseFromData},
    {"usericon", &Horizon::Keys::UserIcon::parseFromData},
    {"usergroups", &Horizon::Keys::UserGroups::parseFromData},

    {"diskid", &Horizon::Keys::DiskId::parseFromData},
    {"disklabel", &Horizon::Keys::DiskLabel::parseFromData},
    {"partition", &Horizon::Keys::Partition::parseFromData},
    {"lvm_pv", &Horizon::Keys::LVMPhysical::parseFromData},
    {"lvm_vg", &Horizon::Keys::LVMGroup::parseFromData},
    {"lvm_lv", &Horizon::Keys::LVMVolume::parseFromData},
    {"encrypt", &Horizon::Keys::Encrypt::parseFromData},
    {"fs", &Horizon::Keys::Filesystem::parseFromData},
    {"mount", &Horizon::Keys::Mount::parseFromData}
};


namespace Horizon {

struct Script::ScriptPrivate {
    /*! Determines whether or not to enable networking. */
    std::unique_ptr<Horizon::Keys::Network> network;
    /*! The target system's hostname. */
    std::unique_ptr<Horizon::Keys::Hostname> hostname;
    /*! The packages to install to the target system. */
    std::set<std::string> packages;
    /*! The root shadow line. */
    std::unique_ptr<Horizon::Keys::RootPassphrase> rootpw;
    /*! Target system's mountpoints. */
    std::vector< std::unique_ptr<Horizon::Keys::Mount> > mounts;

    /*! Network addressing configuration */
    std::vector< std::unique_ptr<Horizon::Keys::NetAddress> > addresses;

    /*! APK repositories */
    std::vector< std::unique_ptr<Horizon::Keys::Repository> > repos;

    /*! Store +key_obj+ representing the key +key_name+.
     * @param key_name      The name of the key that is being stored.
     * @param key_obj       The Key object associated with the key.
     * @param errors        Output parameter: if given, incremented on error.
     * @param warnings      Output parameter: if given, incremented on warning.
     * @param opts          Script parsing options.
     */
    bool store_key(const std::string key_name, Keys::Key *key_obj, int lineno,
                   int *errors, int *warnings, ScriptOptions opts) {
#define DUPLICATE_ERROR(OBJ, KEY, OLD_VAL) \
    std::string err_str("previous value was ");\
    err_str += OLD_VAL;\
    err_str += " at installfile:" + std::to_string(OBJ->lineno());\
    if(errors) *errors += 1;\
    output_error("installfile:" + std::to_string(lineno),\
                 "duplicate value for key '" + KEY + "'", err_str);

        if(key_name == "network") {
            if(this->network) {
                DUPLICATE_ERROR(this->network, std::string("network"),
                                this->network->test() ? "true" : "false")
                return false;
            }
            std::unique_ptr<Keys::Network> net(
                        dynamic_cast<Keys::Network *>(key_obj)
            );
            this->network = std::move(net);
            return true;
        } else if(key_name == "hostname") {
            if(this->hostname) {
                DUPLICATE_ERROR(this->hostname, std::string("hostname"),
                                this->hostname->value())
                return false;
            }
            std::unique_ptr<Keys::Hostname> name(
                        dynamic_cast<Keys::Hostname *>(key_obj)
            );
            this->hostname = std::move(name);
            return true;
        } else if(key_name == "pkginstall") {
            Keys::PkgInstall *install = dynamic_cast<Keys::PkgInstall *>(key_obj);
            for(auto &pkg : install->packages()) {
                if(opts.test(StrictMode) && packages.find(pkg) != packages.end()) {
                    if(warnings) *warnings += 1;
                    output_warning("installfile:" + std::to_string(lineno),
                                   "package '" + pkg + "' has already been specified",
                                   "");
                    continue;
                }
                packages.insert(pkg);
            }
            delete install;
            return true;
        } else if(key_name == "rootpw") {
            if(this->rootpw) {
                DUPLICATE_ERROR(this->rootpw, std::string("rootpw"),
                                "an encrypted passphrase")
                return false;
            }
            std::unique_ptr<Keys::RootPassphrase> name(
                        dynamic_cast<Keys::RootPassphrase *>(key_obj)
            );
            this->rootpw = std::move(name);
            return true;
        } else if(key_name == "mount") {
            std::unique_ptr<Keys::Mount> mount(
                        dynamic_cast<Keys::Mount *>(key_obj)
            );
            this->mounts.push_back(std::move(mount));
            return true;
        } else if(key_name == "netaddress") {
            std::unique_ptr<Keys::NetAddress> addr(
                        dynamic_cast<Keys::NetAddress *>(key_obj)
            );
            this->addresses.push_back(std::move(addr));
            return true;
        } else if(key_name == "repository") {
            std::unique_ptr<Keys::Repository> repo(
                        dynamic_cast<Keys::Repository *>(key_obj)
            );
            this->repos.push_back(std::move(repo));
            return true;
        } else {
            return false;
        }
#undef DUPLICATE_ERROR
    }
};


Script::Script() {
    internal = new ScriptPrivate;
}

Script::~Script() {
    delete internal;
}

const Script *Script::load(const std::string path, const ScriptOptions opts) {
    std::ifstream file(path);
    if(!file) {
        output_error(path, "Cannot open installfile", "");
        return nullptr;
    }

    return Script::load(file, opts);
}

#define PARSER_ERROR(err_str) \
    errors++;\
    output_error("installfile:" + std::to_string(lineno),\
                 err_str, "");\
    if(!opts.test(ScriptOptionFlags::KeepGoing)) {\
        break;\
    }

#define PARSER_WARNING(warn_str) \
    warnings++;\
    output_warning("installfile:" + std::to_string(lineno),\
                   warn_str, "");

const Script *Script::load(std::istream &sstream, const ScriptOptions opts) {
    using namespace Horizon::Keys;
    Script *the_script = new Script;

    int lineno = 0;
    char nextline[LINE_MAX];
    const std::string delim(" \t");
    int errors = 0, warnings = 0;

    while(sstream.getline(nextline, sizeof(nextline))) {
        lineno++;
        if(nextline[0] == '#') {
            /* This is a comment line; ignore it. */
            continue;
        }

        const std::string line(nextline);
        std::string key;
        std::string::size_type start, key_end, value_begin;
        start = line.find_first_not_of(delim);
        if(start == std::string::npos) {
            /* This is a blank line; ignore it. */
            continue;
        }

        key_end = line.find_first_of(delim, start);
        value_begin = line.find_first_not_of(delim, key_end);
        key = line.substr(start, (key_end - start));
        if(key_end == std::string::npos || value_begin == std::string::npos) {
            /* Key without value */
            PARSER_ERROR("key '" + key + "' has no value")
            continue;
        }

        /* Normalise key to lower-case */
        std::transform(key.begin(), key.end(), key.begin(), ::tolower);

        if(valid_keys.find(key) == valid_keys.end()) {
            /* Invalid key */
            if(opts.test(StrictMode)) {
                PARSER_ERROR("key '" + key + "' is not defined")
            } else {
                PARSER_WARNING("key '" + key + "' is not defined")
            }
            continue;
        }

        Key *key_obj = valid_keys.at(key)(line.substr(value_begin), lineno,
                                          &errors, &warnings);
        if(!key_obj) {
            PARSER_ERROR("value for key '" + key + "' was invalid")
            continue;
        }

        if(!the_script->internal->store_key(key, key_obj, lineno, &errors,
                                            &warnings, opts)) {
            PARSER_ERROR("stopping due to prior errors")
            continue;
        }
    }

    if(sstream.fail() && !sstream.eof()) {
        output_error("installfile:" + std::to_string(lineno + 1),
                     "line exceeds maximum length",
                     "Maximum line length is " + std::to_string(LINE_MAX));
        errors++;
    }

    if(sstream.bad() && !sstream.eof()) {
        output_error("installfile:" + std::to_string(lineno),
                     "I/O error while reading installfile", "");
        errors++;
    }

    /* Ensure all required keys are present. */
#define MISSING_ERROR(key) \
    output_error("installfile:" + std::to_string(lineno),\
                 "expected value for key '" + std::string(key) + "'",\
                 "this key is required");\
    errors++;

    if(errors == 0) {
        if(!the_script->internal->network) {
            MISSING_ERROR("network")
        }
        if(!the_script->internal->hostname) {
            MISSING_ERROR("hostname")
        }
        if(the_script->internal->packages.size() == 0) {
            MISSING_ERROR("pkginstall")
        }
        if(!the_script->internal->rootpw) {
            MISSING_ERROR("rootpw")
        }
        if(the_script->internal->mounts.size() == 0) {
            MISSING_ERROR("mount")
        }
    }
#undef MISSING_ERROR

    output_log("parser", "0", "installfile",
               std::to_string(errors) + " error(s), " +
               std::to_string(warnings) + " warning(s).", "");

    if(errors > 0) {
        delete the_script;
        return nullptr;
    } else {
        the_script->opts = opts;
        return the_script;
    }
}

bool Script::validate() const {
    int failures = 0;
    std::set<std::string> seen_mounts;
    std::map<const std::string, int> seen_iface;

    if(!this->internal->network->validate(this->opts)) failures++;
    if(!this->internal->hostname->validate(this->opts)) failures++;
    if(!this->internal->rootpw->validate(this->opts)) failures++;

    for(auto &mount : this->internal->mounts) {
        if(!mount->validate(this->opts)) {
            failures++;
            continue;
        }

        /* Runner.Validate.mount.Unique */
        if(seen_mounts.find(mount->mountpoint()) != seen_mounts.end()) {
            failures++;
            output_error("installfile:" + std::to_string(mount->lineno()),
                         "mount: mountpoint " + mount->mountpoint() +
                         " has already been specified; " + mount->device() +
                         " is a duplicate");
        }
        seen_mounts.insert(mount->mountpoint());
        if(this->opts.test(InstallEnvironment)) {
            /* TODO: Runner.Validate.mount.Block for not-yet-created devs. */
        }
    }

    /* Runner.Validate.mount.Root */
    if(seen_mounts.find("/") == seen_mounts.end()) {
        failures++;
        output_error("installfile:0", "mount: no root mount specified");
    }

    /* Runner.Validate.network.netaddress */
    if(this->internal->network->test() &&
       this->internal->addresses.size() == 0) {
        failures++;
        output_error("installfile:0",
                     "networking requested but no 'netaddress' defined",
                     "You need to specify at least one address to enable "
                     "networking.");
    }

    for(auto &address : this->internal->addresses) {
        if(!address->validate(this->opts)) {
            failures++;
        }

        /* Runner.Validate.network.netaddress.Count */
        if(seen_iface.find(address->iface()) == seen_iface.end()) {
            seen_iface.insert(std::make_pair(address->iface(), 1));
        } else {
            seen_iface[address->iface()] += 1;
            if(seen_iface[address->iface()] > 255) {
                failures++;
                output_error("installfile:" + std::to_string(address->lineno()),
                             "netaddress: interface '" + address->iface() +
                             "' has too many addresses assigned");
            }
        }
    }

    if(this->internal->repos.size() == 0) {
        Keys::Repository *sys_key = dynamic_cast<Keys::Repository *>(
            Horizon::Keys::Repository::parseFromData(
                "https://distfiles.adelielinux.org/adelie/stable/system", 0,
                nullptr, nullptr
            )
        );
        if(!sys_key) {
            output_error("internal", "failed to create default system repository");
            return false;
        }
        std::unique_ptr<Keys::Repository> sys_repo(sys_key);
        this->internal->repos.push_back(std::move(sys_repo));
        Keys::Repository *user_key = dynamic_cast<Keys::Repository *>(
            Horizon::Keys::Repository::parseFromData(
                "https://distfiles.adelielinux.org/adelie/stable/user", 0,
                nullptr, nullptr
            )
        );
        if(!user_key) {
            output_error("internal", "failed to create default user repository");
            return false;
        }
        std::unique_ptr<Keys::Repository> user_repo(user_key);
        this->internal->repos.push_back(std::move(user_repo));
    }

    for(auto &repo : this->internal->repos) {
        if(!repo->validate(this->opts)) {
            failures++;
        }
    }

    if(this->internal->repos.size() > 10) {
        failures++;
        output_error("installfile:" +
                     std::to_string(this->internal->repos[11]->lineno()),
                     "repository: too many repositories specified",
                     "You may only specify up to 10 repositories.");
    }

    output_log("validator", "0", "installfile",
               std::to_string(failures) + " failure(s).", "");
    return (failures == 0);
}

bool Script::execute() const {
    bool success;

    /* Runner.Execute.Verify */
    output_step_start("validate");
    success = this->validate();
    output_step_end("validate");
    if(!success) {
        /* Runner.Execute.Verify.Failure */
        output_error("validator", "The HorizonScript failed validation",
                     "Check the output from the validator.");
        return false;
    }

#define EXECUTE_FAILURE(phase) \
    output_error(phase, "The HorizonScript failed to execute",\
                 "Check the log file for more details.")

    /**************** DISK SETUP ****************/
    output_step_start("disk");
    /* Sort by mountpoint.
     * This ensures that any subdirectory mounts come after their parent. */
    std::sort(this->internal->mounts.begin(), this->internal->mounts.end(),
              [](std::unique_ptr<Keys::Mount> const &e1,
                 std::unique_ptr<Keys::Mount> const &e2) {
        return e1->mountpoint() < e2->mountpoint();
    });
    for(auto &mount : this->internal->mounts) {
        if(!mount->execute(opts)) {
            EXECUTE_FAILURE("disk");
            return false;
        }
    }
    output_step_end("disk");

    /**************** PRE PACKAGE METADATA ****************/
    output_step_start("pre-metadata");
    if(!this->internal->hostname->execute(opts)) {
        EXECUTE_FAILURE("pre-metadata");
        return false;
    }
    output_step_end("pre-metadata");

    /**************** PKGDB ****************/
    output_step_start("pkgdb");
    for(auto &repo : this->internal->repos) {
        if(!repo->execute(opts)) {
            EXECUTE_FAILURE("pkgdb");
            return false;
        }
    }

    /* Runner.Execute.pkginstall.APKDB */
    output_info("internal", "initialising APK");
    if(opts.test(Simulate)) {
        std::cout << "apk --root /target --initdb add" << std::endl;
    } else {
        if(system("apk --root /target --initdb add") != 0) {
            EXECUTE_FAILURE("pkgdb");
            return false;
        }
    }

    /* Runner.Execute.pkginstall */
    output_info("internal", "installing packages to target");
    std::ostringstream pkg_list;
    for(auto &pkg : this->internal->packages) {
        pkg_list << pkg << " ";
    }
    if(opts.test(Simulate)) {
        std::cout << "apk --root /target update" << std::endl;
        std::cout << "apk --root /target add " << pkg_list.str() << std::endl;
    } else {
        if(system("apk --root /target update") != 0) {
            EXECUTE_FAILURE("pkgdb");
            return false;
        }
        std::string apk_invoc = "apk --root /target add " + pkg_list.str();
        if(system(apk_invoc.c_str()) != 0) {
            EXECUTE_FAILURE("pkgdb");
            return false;
        }
    }

    output_step_end("pkgdb");

    /**************** POST PACKAGE METADATA ****************/
    output_step_start("post-metadata");
    if(!this->internal->rootpw->execute(opts)) {
        EXECUTE_FAILURE("post-metadata");
        return false;
    }
    output_step_end("post-metadata");
    return true;
}

}
