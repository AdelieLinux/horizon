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
#include "util/filesystem.hh"
#include <fstream>
#include <iostream>
#include <map>
#include <set>
#include <sstream>

#include "script.hh"
#include "script_i.hh"
#include "script_l.hh"
#include "disk.hh"
#include "meta.hh"
#include "network.hh"
#include "user.hh"

#include "util/output.hh"

#define SCRIPT_LINE_MAX 512


typedef Horizon::Keys::Key *(*key_parse_fn)(const std::string &,
                                            const Horizon::ScriptLocation &,
                                            int*, int*, const Horizon::Script*);

using namespace Horizon::Keys;

const std::map<std::string, key_parse_fn> valid_keys = {
    {"network", &Network::parseFromData},
    {"hostname", &Hostname::parseFromData},
    {"pkginstall", &PkgInstall::parseFromData},
    {"rootpw", &RootPassphrase::parseFromData},

    {"arch", &Arch::parseFromData},
    {"language", &Language::parseFromData},
    {"keymap", &Keymap::parseFromData},
    {"firmware", &Firmware::parseFromData},
    {"timezone", &Timezone::parseFromData},
    {"repository", &Repository::parseFromData},
    {"signingkey", &SigningKey::parseFromData},
    {"svcenable", &SvcEnable::parseFromData},

    {"netconfigtype", &NetConfigType::parseFromData},
    {"netaddress", &NetAddress::parseFromData},
    {"nameserver", &Nameserver::parseFromData},
    {"netssid", &NetSSID::parseFromData},

    {"username", &Username::parseFromData},
    {"useralias", &UserAlias::parseFromData},
    {"userpw", &UserPassphrase::parseFromData},
    {"usericon", &UserIcon::parseFromData},
    {"usergroups", &UserGroups::parseFromData},

    {"diskid", &DiskId::parseFromData},
    {"disklabel", &DiskLabel::parseFromData},
    {"partition", &Partition::parseFromData},
    {"lvm_pv", &LVMPhysical::parseFromData},
    {"lvm_vg", &LVMGroup::parseFromData},
    {"lvm_lv", &LVMVolume::parseFromData},
    {"encrypt", &Encrypt::parseFromData},
    {"fs", &Filesystem::parseFromData},
    {"mount", &Mount::parseFromData}
};


namespace Horizon {

bool Script::ScriptPrivate::store_key(const std::string &key_name, Key *obj,
                                      const ScriptLocation &pos, int *errors,
                                      int *warnings, const ScriptOptions &opts) {
    if(key_name == "network") {
        return store_network(obj, pos, errors, warnings, opts);
    } else if(key_name == "netconfigtype") {
        return store_netconfig(obj, pos, errors, warnings, opts);
    } else if(key_name == "netaddress") {
        std::unique_ptr<NetAddress> addr(dynamic_cast<NetAddress *>(obj));
        this->addresses.push_back(std::move(addr));
        return true;
    } else if(key_name == "nameserver") {
        std::unique_ptr<Nameserver> ns(dynamic_cast<Nameserver *>(obj));
        this->nses.push_back(std::move(ns));
        return true;
    } else if(key_name == "netssid") {
        std::unique_ptr<NetSSID> ssid(dynamic_cast<NetSSID *>(obj));
        this->ssids.push_back(std::move(ssid));
        return true;
    } else if(key_name == "hostname") {
        return store_hostname(obj, pos, errors, warnings, opts);
    } else if(key_name == "pkginstall") {
        return store_pkginstall(obj, pos, errors, warnings, opts);
    } else if(key_name == "arch") {
        return store_arch(obj, pos, errors, warnings, opts);
    } else if(key_name == "rootpw") {
        return store_rootpw(obj, pos, errors, warnings, opts);
    } else if(key_name == "language") {
        return store_lang(obj, pos, errors, warnings, opts);
    } else if(key_name == "keymap") {
        return store_keymap(obj, pos, errors, warnings, opts);
    } else if(key_name == "firmware") {
        return store_firmware(obj, pos, errors, warnings, opts);
    } else if(key_name == "timezone") {
        return store_timezone(obj, pos, errors, warnings, opts);
    } else if(key_name == "repository") {
        std::unique_ptr<Repository> repo(dynamic_cast<Repository *>(obj));
        this->repos.push_back(std::move(repo));
        return true;
    } else if(key_name == "signingkey") {
        std::unique_ptr<SigningKey> key(dynamic_cast<SigningKey *>(obj));
        this->repo_keys.push_back(std::move(key));
        return true;
    } else if(key_name == "svcenable") {
        return store_svcenable(obj, pos, errors, warnings, opts);
    } else if(key_name == "username") {
        return store_username(obj, pos, errors, warnings, opts);
    } else if(key_name == "useralias") {
        return store_useralias(obj, pos, errors, warnings, opts);
    } else if(key_name == "userpw") {
        return store_userpw(obj, pos, errors, warnings, opts);
    } else if(key_name == "usericon") {
        return store_usericon(obj, pos, errors, warnings, opts);
    } else if(key_name == "usergroups") {
        return store_usergroups(obj, pos, errors, warnings, opts);
    } else if(key_name == "diskid") {
        std::unique_ptr<DiskId> diskid(dynamic_cast<DiskId *>(obj));
        this->diskids.push_back(std::move(diskid));
        return true;
    } else if(key_name == "disklabel") {
        std::unique_ptr<DiskLabel> l(dynamic_cast<DiskLabel *>(obj));
        this->disklabels.push_back(std::move(l));
        return true;
    } else if(key_name == "partition") {
        std::unique_ptr<Partition> p(dynamic_cast<Partition *>(obj));
        this->partitions.push_back(std::move(p));
        return true;
    } else if(key_name == "lvm_pv") {
        std::unique_ptr<LVMPhysical> pv(dynamic_cast<LVMPhysical *>(obj));
        this->lvm_pvs.push_back(std::move(pv));
        return true;
    } else if(key_name == "lvm_vg") {
        std::unique_ptr<LVMGroup> vg(dynamic_cast<LVMGroup *>(obj));
        this->lvm_vgs.push_back(std::move(vg));
        return true;
    } else if(key_name == "lvm_lv") {
        std::unique_ptr<LVMVolume> lv(dynamic_cast<LVMVolume *>(obj));
        this->lvm_lvs.push_back(std::move(lv));
        return true;
    } else if(key_name == "encrypt") {
        std::unique_ptr<Encrypt> e(dynamic_cast<Encrypt *>(obj));
        this->luks.push_back(std::move(e));
        return true;
    } else if(key_name == "fs") {
        std::unique_ptr<Filesystem> fs(dynamic_cast<Filesystem *>(obj));
        this->fses.push_back(std::move(fs));
        return true;
    } else if(key_name == "mount") {
        std::unique_ptr<Mount> mount(dynamic_cast<Mount *>(obj));
        this->mounts.push_back(std::move(mount));
        return true;
    } else {
        return false;  /* LCOV_EXCL_LINE - only here for error prevention */
    }
}


Script::Script() {
    internal = new ScriptPrivate;
    internal->target = "/target";
}

Script::~Script() {
    delete internal;
}

Script *Script::load(const std::string &path, const ScriptOptions &opts) {
    std::ifstream file(path);
    if(!file) {
        output_error(path, "Cannot open installfile", "");
        return nullptr;
    }

    return Script::load(file, opts, path);
}


Script *Script::load(std::istream &sstream, const ScriptOptions &opts,
                     const std::string &name) {
#define PARSER_ERROR(err_str) \
    errors++;\
    output_error(pos, err_str, "");\
    if(!opts.test(ScriptOptionFlags::KeepGoing)) {\
        break;\
    }

#define PARSER_WARNING(warn_str) \
    warnings++;\
    output_warning(pos, warn_str, "");

    using namespace Horizon::Keys;

    Script *the_script = new Script;
    the_script->opts = opts;

    int lineno = 0;
    char nextline[SCRIPT_LINE_MAX];
    const std::string delim(" \t");
    int errors = 0, warnings = 0;

    while(sstream.getline(nextline, sizeof(nextline))) {
        lineno++;
        if(nextline[0] == '#') {
            /* This is a comment line; ignore it. */
            continue;
        }

        const ScriptLocation pos(name, lineno, false);
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

        Key *key_obj = valid_keys.at(key)(line.substr(value_begin), pos,
                                          &errors, &warnings, the_script);
        if(!key_obj) {
            PARSER_ERROR("value for key '" + key + "' was invalid")
            continue;
        }

        if(!the_script->internal->store_key(key, key_obj, pos, &errors,
                                            &warnings, opts)) {
            PARSER_ERROR("stopping due to prior errors")
            continue;
        }
    }

    if(sstream.fail() && !sstream.eof()) {
        output_error("installfile:" + std::to_string(lineno + 1),
                     "line exceeds maximum length",
                     "Maximum line length is " +
                     std::to_string(SCRIPT_LINE_MAX));
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
        if(the_script->internal->mounts.size() == 0 && !opts.test(ImageOnly)) {
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
        return the_script;
    }

#undef PARSER_WARNING
#undef PARSER_ERROR
}

/* LCOV_EXCL_START */
const std::string Script::targetDirectory() const {
    return this->internal->target;
}

void Script::setTargetDirectory(const std::string &dir) {
    this->internal->target = dir;
}

const Keys::Key *Script::getOneValue(std::string name) const {
    if(name == "network") {
        return this->internal->network.get();
    } else if(name == "netconfigtype") {
        return this->internal->netconfig.get();
    } else if(name == "hostname") {
        return this->internal->hostname.get();
    } else if(name == "arch") {
        return this->internal->arch.get();
    } else if(name == "rootpw") {
        return this->internal->rootpw.get();
    } else if(name == "language") {
        return this->internal->lang.get();
    } else if(name == "keymap") {
        return this->internal->keymap.get();
    } else if(name == "firmware") {
#ifdef NON_LIBRE_FIRMWARE
        return this->internal->firmware.get();
#else  /* !NON_LIBRE_FIRMWARE */
        return nullptr;
#endif  /* NON_LIBRE_FIRMWARE */
    } else if(name == "timezone") {
        return this->internal->tzone.get();
    }

    assert("Unknown key given to getOneValue." == nullptr);
    return nullptr;
}

const std::vector<Keys::Key *> Script::getValues(std::string name) const {
    std::vector<Keys::Key *> values;

    if(name == "netaddress") {
        for(auto &addr : this->internal->addresses) values.push_back(addr.get());
    } else if(name == "nameserver") {
        for(auto &ns : this->internal->nses) values.push_back(ns.get());
    } else if(name == "netssid") {
        for(auto &ssid : this->internal->ssids) values.push_back(ssid.get());
    } else if(name == "pkginstall") {
        /* XXX */
    } else if(name == "repository") {
        for(auto &repo : this->internal->repos) values.push_back(repo.get());
    } else if(name == "signing_key") {
        for(auto &key : this->internal->repo_keys) values.push_back(key.get());
    } else if(name == "username" || name == "useralias" || name == "userpw" ||
              name == "usericon" || name == "usergroups") {
        /* XXX */
    } else if(name == "diskid") {
        for(auto &id : this->internal->diskids) values.push_back(id.get());
    } else if(name == "disklabel") {
        for(auto &label : this->internal->disklabels) values.push_back(label.get());
    } else if(name == "partition") {
        for(auto &part : this->internal->partitions) values.push_back(part.get());
    } else if(name == "lvm_pv") {
        for(auto &pv : this->internal->lvm_pvs) values.push_back(pv.get());
    } else if(name == "lvm_vg") {
        for(auto &vg : this->internal->lvm_vgs) values.push_back(vg.get());
    } else if(name == "lvm_lv") {
        for(auto &lv : this->internal->lvm_lvs) values.push_back(lv.get());
    } else if(name == "encrypt") {
        /* XXX */
    } else if(name == "fs") {
        for(auto &fs : this->internal->fses) values.push_back(fs.get());
    } else if(name == "mount") {
        for(auto &mnt : this->internal->mounts) values.push_back(mnt.get());
    } else {
        assert("Unknown key given to getValues." == nullptr);
    }

    return values;
}
/* LCOV_EXCL_STOP */

ScriptOptions Script::options() const {
    return this->opts;
}

}
