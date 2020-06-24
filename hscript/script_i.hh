/*
 * script_i.hh - Implementation of internal doodads for the Script class
 * libhscript, the HorizonScript library for
 * Project Horizon
 *
 * Copyright (c) 2019-2020 Adélie Linux and contributors.  All rights reserved.
 * This code is licensed under the AGPL 3.0 license, as noted in the
 * LICENSE-code file in the root directory of this repository.
 *
 * SPDX-License-Identifier: AGPL-3.0-only
 */

#ifndef HSCRIPT_SCRIPT_INTERNAL_HH_
#define HSCRIPT_SCRIPT_INTERNAL_HH_

#include <assert.h>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "script_l.hh"

#include "disk.hh"
#include "meta.hh"
#include "network.hh"
#include "user.hh"

using namespace Horizon::Keys;

namespace Horizon {

/*! Describes a user account. */
struct UserDetail {
    std::unique_ptr<Username> name;
    std::unique_ptr<UserAlias> alias;
    std::unique_ptr<UserPassphrase> passphrase;
    std::unique_ptr<UserIcon> icon;
    std::vector< std::unique_ptr<UserGroups> > groups;
};

struct Script::ScriptPrivate {
    /*! Determines the target directory (usually /target) */
    std::string target;

    /*! Determines whether or not to enable networking. */
    std::unique_ptr<Network> network;
    /*! Determines the network configuration system to use. */
    std::unique_ptr<NetConfigType> netconfig;
    /*! The target system's hostname. */
    std::unique_ptr<Hostname> hostname;
    /*! The packages to install to the target system. */
    std::set<std::string> packages;
    /*! The root shadow line. */
    std::unique_ptr<RootPassphrase> rootpw;
    /*! The system CPU architecture. */
    std::unique_ptr<Arch> arch;
    /*! The system language. */
    std::unique_ptr<Language> lang;
    /*! The system keymap. */
    std::unique_ptr<Keymap> keymap;
    /*! The system timezone. */
    std::unique_ptr<Timezone> tzone;
    /*! The version of Adélie to install. */
    std::unique_ptr<Version> version;
    /*! The desired bootloader configuration. */
    std::unique_ptr<Bootloader> boot;

    /*! Network addressing configuration */
    std::vector< std::unique_ptr<NetAddress> > addresses;
    /*! Network nameserver resolver addresses */
    std::vector< std::unique_ptr<Nameserver> > nses;
    /*! Wireless networking configuration */
    std::vector< std::unique_ptr<NetSSID> > ssids;

    /*! APK repositories */
    std::vector< std::unique_ptr<Repository> > repos;
    /*! APK repository keys */
    std::vector< std::unique_ptr<SigningKey> > repo_keys;

    /*! Services to enable */
    std::vector< std::unique_ptr<SvcEnable> > svcs_enable;

    /*! User account information */
    std::map< std::string, std::unique_ptr<UserDetail> > accounts;

    /*! Disk identification keys */
    std::vector< std::unique_ptr<DiskId> > diskids;
    /*! Disklabel configuration keys */
    std::vector< std::unique_ptr<DiskLabel> > disklabels;
    /*! Partition creation keys */
    std::vector< std::unique_ptr<Partition> > partitions;
    /*! LVM physical volume keys */
    std::vector< std::unique_ptr<LVMPhysical> > lvm_pvs;
    /*! LVM volume group keys */
    std::vector< std::unique_ptr<LVMGroup> > lvm_vgs;
    /*! LVM logical volume keys */
    std::vector< std::unique_ptr<LVMVolume> > lvm_lvs;
    /*! LUKS creation keys */
    std::vector< std::unique_ptr<Encrypt> > luks;
    /*! Filesystem creation keys */
    std::vector< std::unique_ptr<Filesystem> > fses;
    /*! Target system's mountpoints. */
    std::vector< std::unique_ptr<Mount> > mounts;

#ifdef NON_LIBRE_FIRMWARE
    std::unique_ptr<Firmware> firmware;
#endif

    /*! Store +key_obj+ representing the key +key_name+.
     * @param key_name      The name of the key that is being stored.
     * @param obj           The Key object associated with the key.
     * @param pos           The on-disk script position of the key.
     * @param errors        Output parameter: if given, incremented on error.
     * @param warnings      Output parameter: if given, incremented on warning.
     * @param opts          Script parsing options.
     */
    bool store_key(const std::string &key_name, Key *obj,
                   const ScriptLocation &pos, int *errors, int *warnings,
                   const ScriptOptions &opts);

#define DUPLICATE_ERROR(OBJ, KEY, OLD_VAL) \
    if(pos.inherited) return true;\
    std::string err_str("previous value was ");\
    err_str += OLD_VAL;\
    err_str += " at " + OBJ->where().name;\
    err_str += ":" + std::to_string(OBJ->where().line);\
    if(errors) *errors += 1;\
    output_error(pos, "duplicate value for key '" + std::string(KEY) + "'",\
                 err_str);

    bool store_network(Key* obj, const ScriptLocation &pos, int *errors, int *,
                       const ScriptOptions &) {
        if(network) {
            DUPLICATE_ERROR(network, "network",
                            network->test() ? "true" : "false")
            return false;
        }
        std::unique_ptr<Network> net(dynamic_cast<Network *>(obj));
        network = std::move(net);
        return true;
    }

    bool store_netconfig(Key *obj, const ScriptLocation &pos, int *errors,
                         int *, const ScriptOptions &) {
        if(netconfig) {
            DUPLICATE_ERROR(netconfig, "netconfigtype",
                            netconfig->type());
            return false;
        }
        std::unique_ptr<NetConfigType> nc(dynamic_cast<NetConfigType *>(obj));
        netconfig = std::move(nc);
        return true;
    }

    bool store_hostname(Key* obj, const ScriptLocation &pos, int *errors,
                        int *, const ScriptOptions &) {
        if(hostname) {
            DUPLICATE_ERROR(hostname, "hostname", hostname->value())
            return false;
        }
        std::unique_ptr<Hostname> name(dynamic_cast<Hostname *>(obj));
        hostname = std::move(name);
        return true;
    }

    bool store_pkginstall(Key* obj, const ScriptLocation &pos, int *,
                          int *warnings, const ScriptOptions &opts) {
        PkgInstall *install = dynamic_cast<PkgInstall *>(obj);
        for(auto &pkg : install->packages()) {
            if(opts.test(StrictMode) && packages.find(pkg) != packages.end()) {
                if(warnings) *warnings += 1;
                output_warning(pos, "pkginstall: package '" + pkg +
                               "' has already been specified");
                continue;
            }
            packages.insert(pkg);
        }
        delete install;
        return true;
    }

    bool store_arch(Key* obj, const ScriptLocation &pos, int *errors, int *,
                    const ScriptOptions &) {
        if(arch) {
            DUPLICATE_ERROR(arch, "arch", arch->value())
            return false;
        }
        std::unique_ptr<Arch> a(dynamic_cast<Arch *>(obj));
        arch = std::move(a);
        return true;
    }

    bool store_rootpw(Key* obj, const ScriptLocation &pos, int *errors, int *,
                      const ScriptOptions &) {
        if(rootpw) {
            DUPLICATE_ERROR(rootpw, "rootpw", "an encrypted passphrase")
            return false;
        }
        std::unique_ptr<RootPassphrase> r(dynamic_cast<RootPassphrase *>(obj));
        rootpw = std::move(r);
        return true;
    }

    bool store_firmware(Key *obj, const ScriptLocation &pos, int *errors, int *,
                        const ScriptOptions &) {
        std::unique_ptr<Firmware> f(dynamic_cast<Firmware *>(obj));
#ifdef NON_LIBRE_FIRMWARE
        if(firmware) {
            DUPLICATE_ERROR(firmware, "firmware",
                            (firmware->test()) ? "true" : "false")
            return false;
        }
        firmware = std::move(f);
        return true;
#else
        assert(!f->test());
        return true;
#endif
    }

    bool store_lang(Key *obj, const ScriptLocation &pos, int *errors, int *,
                    const ScriptOptions &) {
        if(lang) {
            DUPLICATE_ERROR(lang, "language", lang->value())
            return false;
        }
        std::unique_ptr<Language> l(dynamic_cast<Language *>(obj));
        lang = std::move(l);
        return true;
    }

    bool store_keymap(Key *obj, const ScriptLocation &pos, int *errors, int *,
                      const ScriptOptions &) {
        if(keymap) {
            DUPLICATE_ERROR(keymap, "keymap", keymap->value())
            return false;
        }
        std::unique_ptr<Keymap> k(dynamic_cast<Keymap *>(obj));
        keymap = std::move(k);
        packages.insert("kbd-keymaps");
        return true;
    }

    bool store_timezone(Key *obj, const ScriptLocation &pos, int *errors, int *,
                        const ScriptOptions &) {
        if(tzone) {
            DUPLICATE_ERROR(tzone, "timezone", tzone->value())
            return false;
        }
        std::unique_ptr<Timezone> t(dynamic_cast<Timezone *>(obj));
        tzone = std::move(t);
        return true;
    }

    bool store_svcenable(Key *obj, const ScriptLocation &pos, int *, int *warn,
                         const ScriptOptions &) {
        std::unique_ptr<SvcEnable> svc(dynamic_cast<SvcEnable *>(obj));
        for(const auto &s : svcs_enable) {
            if(s->service() == svc->service()) {
                if(warn) *warn += 1;
                output_warning(pos, "svcenable: service already enabled",
                               s->service());
                return true;
            }
        }

        svcs_enable.push_back(std::move(svc));
        return true;
    }

    bool store_version(Key *obj, const ScriptLocation &pos, int *errors, int *,
                       const ScriptOptions &) {
        if(version) {
            DUPLICATE_ERROR(version, "version", version->value())
            return false;
        }
        std::unique_ptr<Version> v(dynamic_cast<Version *>(obj));
        version = std::move(v);
        return true;
    }

    bool store_bootloader(Key *obj, const ScriptLocation &pos, int *errors,
                          int *, const ScriptOptions &) {
        if(boot) {
            DUPLICATE_ERROR(boot, "bootloader", boot->value())
            return false;
        }
        std::unique_ptr<Bootloader> b(dynamic_cast<Bootloader *>(obj));
        boot = std::move(b);
        return true;
    }

    bool store_username(Key *obj, const ScriptLocation &pos, int *errors, int *,
                        const ScriptOptions &) {
        if(accounts.size() >= 255) {
            if(errors) *errors += 1;
            output_error(pos, "username: too many users",
                         "you may only specify 255 users");
            return false;
        }
        std::unique_ptr<Username> name(dynamic_cast<Username *>(obj));
        if(accounts.find(name->value()) != accounts.end()) {
            DUPLICATE_ERROR((*accounts.find(name->value())).second->name,
                            "username", "assigned")
            return false;
        }
        std::unique_ptr<UserDetail> detail(new UserDetail);
        detail->name = std::move(name);
        accounts.insert(std::make_pair(detail->name->value(),
                                       std::move(detail)));
        return true;
    }

#define GET_USER_DETAIL(OBJ, KEY) \
    if(accounts.find(OBJ->username()) == accounts.end()) {\
        if(errors) *errors += 1;\
        output_error(pos, std::string(KEY) + ": account name " +\
                     OBJ->username() + " is unknown");\
        return false;\
    }\
    UserDetail *detail = (*accounts.find(OBJ->username())).second.get();

    bool store_useralias(Key* obj, const ScriptLocation &pos, int *errors,
                         int *, const ScriptOptions &) {
        std::unique_ptr<UserAlias> alias(dynamic_cast<UserAlias *>(obj));
        GET_USER_DETAIL(alias, "useralias")
        /* REQ: Runner.Validate.useralias.Unique */
        if(detail->alias) {
            DUPLICATE_ERROR(detail->alias, "useralias", detail->alias->alias())
            return false;
        }
        detail->alias = std::move(alias);
        return true;
    }

    bool store_userpw(Key *obj, const ScriptLocation &pos, int *errors, int *,
                      const ScriptOptions &) {
        std::unique_ptr<UserPassphrase> pw(dynamic_cast<UserPassphrase *>(obj));
        GET_USER_DETAIL(pw, "userpw")
        /* REQ: Runner.Validate.userpw.Unique */
        if(detail->passphrase) {
            DUPLICATE_ERROR(detail->passphrase, "userpw",
                            "an encrypted passphrase")
            return false;
        }
        detail->passphrase = std::move(pw);
        return true;
    }

    bool store_usericon(Key *obj, const ScriptLocation &pos, int *errors, int *,
                        const ScriptOptions &) {
        std::unique_ptr<UserIcon> icon(dynamic_cast<UserIcon *>(obj));
        GET_USER_DETAIL(icon, "usericon")
        /* REQ: Runner.Validate.usericon.Unique */
        if(detail->icon) {
            DUPLICATE_ERROR(detail->icon, "usericon", detail->icon->icon())
            return false;
        }
        detail->icon = std::move(icon);
        return true;
    }

    bool store_usergroups(Key* obj, const ScriptLocation &pos, int *errors,
                          int *, const ScriptOptions &) {
        std::unique_ptr<UserGroups> grp(dynamic_cast<UserGroups *>(obj));
        GET_USER_DETAIL(grp, "usergroups")
        detail->groups.push_back(std::move(grp));
        return true;
    }
#undef GET_USER_DETAIL

#undef DUPLICATE_ERROR
};

}

#endif /* !HSCRIPT_SCRIPT_INTERNAL_HH_ */
