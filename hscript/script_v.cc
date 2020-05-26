/*
 * script_v.cc - Implementation of global HorizonScript validation routines
 * libhscript, the HorizonScript library for
 * Project Horizon
 *
 * Copyright (c) 2019-2020 Adélie Linux and contributors.  All rights reserved.
 * This code is licensed under the AGPL 3.0 license, as noted in the
 * LICENSE-code file in the root directory of this repository.
 *
 * SPDX-License-Identifier: AGPL-3.0-only
 */

#include <algorithm>
#include <map>
#include <memory>
#ifdef HAS_INSTALL_ENV
#   include <resolv.h>      /* MAXNS */
#else
#   define MAXNS 3          /* default */
#endif
#include <set>
#include <string>
#include <vector>

#include "script.hh"
#include "script_i.hh"
#include "disk.hh"
#include "meta.hh"
#include "network.hh"
#include "user.hh"

#include "util/filesystem.hh"
#include "util/output.hh"

using namespace Horizon::Keys;
using Horizon::ScriptOptions;

using std::to_string;

namespace Horizon {

/*! Perform all necessary validations on a single user account.
 * @param name      The username of the account.
 * @param detail    The UserDetail record of the account.
 * @param opts      The ScriptOptions in use.
 * @returns A count of errors encountered, or 0 if the account is valid.
 */
int validate_one_account(const std::string &name, UserDetail *detail) {
    int failures = 0;

    /* REQ: Runner.Validate.username */
    if(!detail->name->validate()) failures++;

    /* REQ: Runner.Validate.useralias */
    if(detail->alias && !detail->alias->validate()) failures++;

    /* REQ: Runner.Validate.userpw */
    if(detail->passphrase && !detail->passphrase->validate()) failures++;

    /* REQ: Runner.Validate.userpw.None */
    if(!detail->passphrase) {
        output_warning(detail->name->where(),
                       "username: " + name + " has no set passphrase",
                       "This account will not be able to log in.");
    }

    /* REQ: Runner.Validate.usericon */
    if(detail->icon && !detail->icon->validate()) failures++;

    if(detail->groups.size() > 0) {
        std::set<std::string> seen_groups;
        for(auto &group : detail->groups) {
            /* REQ: Runner.Validate.usergroups */
            if(!group->validate()) failures++;

            /* REQ: Runner.Validate.usergroups.Unique */
            const std::set<std::string> these = group->groups();
            if(!std::all_of(these.begin(), these.end(),
                [&seen_groups](std::string elem) {
                    return seen_groups.find(elem) == seen_groups.end();
                })
            ) {
                output_error(group->where(),
                             "usergroups: duplicate group name specified");
                failures++;
            }
            seen_groups.insert(these.begin(), these.end());
        }

        /* REQ: Runner.Validate.usergroups.Count */
        if(seen_groups.size() > 16) {
            output_error("installfile:0", "usergroups: " + name +
                         " is a member of more than 16 groups");
            failures++;
        }
    }

    return failures;
}


/*! Add the default repositories to the repo list.
 * @param repos     The list of repositories.
 * @param firmware  Whether to include firmware repository.
 * The list +repos+ will be modified with the default repositories for
 * Adélie Linux.  Both system/ and user/ will be added.
 */
bool add_default_repos(std::vector<std::unique_ptr<Repository>> &repos,
                       const Script *s, bool firmware = false) {
    Repository *sys_key = dynamic_cast<Repository *>(
        Repository::parseFromData(
            "https://distfiles.adelielinux.org/adelie/stable/system", {"", 0},
            nullptr, nullptr, s
        )
    );
    if(!sys_key) {
        output_error("internal", "failed to create default system repository");
        return false;
    }
    std::unique_ptr<Repository> sys_repo(sys_key);
    repos.push_back(std::move(sys_repo));
    Repository *user_key = dynamic_cast<Repository *>(
        Repository::parseFromData(
            "https://distfiles.adelielinux.org/adelie/stable/user", {"", 0},
            nullptr, nullptr, s
        )
    );
    if(!user_key) {
        output_error("internal", "failed to create default user repository");
        return false;
    }
    std::unique_ptr<Repository> user_repo(user_key);
    repos.push_back(std::move(user_repo));

#ifdef NON_LIBRE_FIRMWARE
    /* REQ: Runner.Execute.firmware.Repository */
    if(firmware) {
        Repository *fw_key = dynamic_cast<Repository *>(
            Repository::parseFromData(
                "https://distfiles.apkfission.net/adelie-stable/nonfree",
                0, nullptr, nullptr, s
            )
        );
        if(!fw_key) {
            output_error("internal",
                         "failed to create firmware repository");
            return false;
        }
        std::unique_ptr<Repository> fw_repo(fw_key);
        repos.push_back(std::move(fw_repo));
    }
#endif  /* NON_LIBRE_FIRMWARE */
    return true;
}


/*! Add the default repository keys to the signing key list.
 * @param keys      The list of repository keys.
 * The list +keys+ will be modified with the default repository signing keys
 * for Adélie Linux.
 */
bool add_default_repo_keys(std::vector<std::unique_ptr<SigningKey>> &keys,
                           const Script *s, bool firmware = false) {
    SigningKey *key = dynamic_cast<SigningKey *>(
        SigningKey::parseFromData(
            "/etc/apk/keys/packages@adelielinux.org.pub", {"", 0},
            nullptr, nullptr, s)
    );
    if(!key) {
        output_error("internal", "failed to create default repository signing key");
        return false;
    }
    std::unique_ptr<SigningKey> repo_key(key);
    keys.push_back(std::move(repo_key));

#ifdef NON_LIBRE_FIRMWARE
    /* REQ: Runner.Execute.signingkey.Firmware */
    if(firmware) {
        SigningKey *fkey = dynamic_cast<SigningKey *>(SigningKey::parseFromData(
            "/etc/apk/keys/packages@pleroma.apkfission.net-5ac0b300.rsa.pub",
                                                  {"", 0}, nullptr, nullptr, s)
        );
        if(!fkey) {
            output_error("internal", "failed to create firmware signing key");
            return false;
        }
        std::unique_ptr<SigningKey> fw_key(fkey);
        keys.push_back(std::move(fw_key));
        fkey = dynamic_cast<SigningKey *>(SigningKey::parseFromData(
            "/etc/apk/keys/packages@pleroma.apkfission.net-5ac04808.rsa.pub",
                                              {"", 0}, nullptr, nullptr, s));
        if(fkey) {
            std::unique_ptr<SigningKey> fw_key2(fkey);
            keys.push_back(std::move(fw_key2));
        }
    }
#endif  /* NON_LIBRE_FIRMWARE */
    return true;
}


bool Horizon::Script::validate() const {
    int failures = 0;
    std::set<std::string> seen_diskids, seen_labels, seen_parts, seen_pvs,
            seen_vg_names, seen_vg_pvs, seen_lvs, seen_fses, seen_mounts,
            seen_luks;
    std::map<const std::string, int> seen_iface;
#ifdef HAS_INSTALL_ENV
    error_code ec;
#endif /* HAS_INSTALL_ENV */

    /* REQ: Runner.Validate.network */
    if(!internal->network->validate()) failures++;

    /* REQ: Runner.Validate.network.netaddress */
    if(internal->network->test() && internal->addresses.size() == 0) {
        failures++;
        output_error("installfile:0", "networking requires 'netaddress'",
                     "You need to specify at least one address to enable "
                     "networking.");
    }
    for(auto &address : internal->addresses) {
        if(!address->validate()) failures++;

        /* REQ: Runner.Validate.network.netaddress.Count */
        if(seen_iface.find(address->iface()) == seen_iface.end()) {
            seen_iface.insert(std::make_pair(address->iface(), 1));
        } else {
            seen_iface[address->iface()] += 1;
            if(seen_iface[address->iface()] > 255) {
                failures++;
                output_error(address->where(), "netaddress: interface '" +
                             address->iface() + "' has too many addresses");
            }
        }
    }

    /* REQ: Runner.Validate.nameserver */
    for(auto &ns : internal->nses) {
        if(!ns->validate()) failures++;
    }
    if(internal->nses.size() > MAXNS) {
        output_warning(internal->nses[MAXNS]->where(),
                       "nameserver: more nameservers are defined than usable",
                       to_string(MAXNS) + " nameservers are allowed");
    }

    /* REQ: Runner.Validate.network.netssid */
    for(auto &ssid : internal->ssids) {
        if(!ssid->validate()) failures++;
    }

    /* REQ: Runner.Validate.hostname */
    if(!internal->hostname->validate()) failures++;

    /* REQ: Runner.Validate.rootpw */
    if(!internal->rootpw->validate()) failures++;

    /* REQ: Runner.Validate.arch */
    if(internal->arch && !internal->arch->validate()) failures++;

    /* REQ: Runner.Validate.language */
    if(internal->lang && !internal->lang->validate()) failures++;

    /* REQ: Runner.Validate.keymap */
    if(internal->keymap && !internal->keymap->validate()) failures++;

#ifdef NON_LIBRE_FIRMWARE
    /* REQ: Runner.Validate.firmware */
    if(internal->firmware && !internal->firmware->validate()) failures++;
#endif

    /* REQ: Runner.Execute.timezone */
    if(!internal->tzone) {
        Timezone *utc = dynamic_cast<Timezone *>
            (Timezone::parseFromData("UTC", {"", 0}, &failures, nullptr, this));
        if(!utc) {
            output_error("internal", "failed to create default timezone");
            return false;
        }
        std::unique_ptr<Timezone> zone(utc);
        internal->tzone = std::move(zone);
    }

    /* REQ: Runner.Validate.timezone */
    if(!internal->tzone->validate()) failures++;

    /* REQ: Script.repository */
    if(internal->repos.size() == 0) {
        if(!add_default_repos(internal->repos, this
#ifdef NON_LIBRE_FIRMWARE
                              , internal->firmware && internal->firmware->test()
#endif
                              )) {
            return false;
        }
    }

    /* REQ: Runner.Validate.repository */
    for(auto &repo : internal->repos) {
        if(!repo->validate()) failures++;
    }
    if(internal->repos.size() > 10) {
        failures++;
        output_error(internal->repos[11]->where(),
                     "repository: too many repositories specified",
                     "You may only specify up to 10 repositories.");
    }

    /* REQ: Script.signingkey */
    if(internal->repo_keys.size() == 0) {
        if(!add_default_repo_keys(internal->repo_keys, this
#ifdef NON_LIBRE_FIRMWARE
                            , internal->firmware && internal->firmware->test()
#endif
                                  )) {
            return false;
        }
    }

    /* REQ: Runner.Validate.signingkey */
    for(auto &key : internal->repo_keys) {
        if(!key->validate()) failures++;
    }
    if(internal->repo_keys.size() > 10) {
        failures++;
        output_error(internal->repo_keys[11]->where(),
                     "signingkey: too many keys specified",
                     "You may only specify up to 10 repository keys.");
    }

    for(auto &acct : internal->accounts) {
        UserDetail *detail = acct.second.get();
        failures += validate_one_account(acct.first, detail);
    }

#define VALIDATE_OR_SKIP(obj) \
    if(!obj->validate()) {\
        failures++;\
        continue;\
    }

    /* REQ: Runner.Validate.diskid */
    for(auto &diskid : internal->diskids) {
        VALIDATE_OR_SKIP(diskid)

        /* REQ: Runner.Validate.diskid.Unique */
        if(seen_diskids.find(diskid->device()) != seen_diskids.end()) {
            failures++;
            output_error(diskid->where(), "diskid: device " +
                         diskid->device() + " has already been identified");
        }
        seen_diskids.insert(diskid->device());
    }

    /* REQ: Runner.Validate.disklabel */
    for(auto &label : internal->disklabels) {
        VALIDATE_OR_SKIP(label)

        /* REQ: Runner.Validate.disklabel.Unique */
        if(seen_labels.find(label->device()) != seen_labels.end()) {
            failures++;
            output_error(label->where(), "disklabel: device " +
                         label->device() + " already has a label queued");
        } else {
            seen_labels.insert(label->device());
        }
    }

    /* REQ: Runner.Validate.partition */
    for(auto &part : internal->partitions) {
        VALIDATE_OR_SKIP(part)

        /* REQ: Runner.Validate.partition.Unique */
        const std::string &dev = part->device();
        const std::string maybe_p(::isdigit(dev[dev.size() - 1]) ? "p" : "");
        std::string name = dev + maybe_p + to_string(part->partno());
        if(seen_parts.find(name) != seen_parts.end()) {
            failures++;
            output_error(part->where(), "partition: partition #" +
                         to_string(part->partno()) + " already exists on " +
                         part->device());
        } else {
            seen_parts.insert(name);
        }
    }

    /* REQ: Runner.Validate.lvm_pv */
    for(auto &pv : internal->lvm_pvs) {
        VALIDATE_OR_SKIP(pv)

        /* We don't actually have a requirement, but... */
        if(seen_pvs.find(pv->value()) != seen_pvs.end()) {
            failures++;
            output_error(pv->where(), "lvm_pv: a physical volume already "
                                      "exists on device " + pv->value());
        } else {
            seen_pvs.insert(pv->value());
        }

        /* REQ: Runner.Validate.lvm_pv.Block */
        if(opts.test(InstallEnvironment)) {
#ifdef HAS_INSTALL_ENV
            if(!fs::exists(pv->value(), ec) &&
               seen_parts.find(pv->value()) == seen_parts.end()) {
                failures++;
                output_error(pv->where(), "lvm_pv: device " + pv->value() +
                             " does not exist");
            }
#endif /* HAS_INSTALL_ENV */
        }
    }

    /* REQ: Runner.Validate.lvm_vg */
    for(auto &vg : internal->lvm_vgs) {
        VALIDATE_OR_SKIP(vg)

        if(seen_vg_names.find(vg->name()) != seen_vg_names.end()) {
            failures++;
            output_error(vg->where(), "lvm_vg: duplicate volume group name",
                         vg->name() + " already given");
        } else {
            seen_vg_names.insert(vg->name());
        }

        if(seen_vg_pvs.find(vg->pv()) != seen_vg_pvs.end()) {
            failures++;
            output_error(vg->where(), "lvm_vg: a volume group already exists "
                         "on " + vg->pv());
        } else {
            seen_vg_pvs.insert(vg->pv());
        }

        /* REQ: Runner.Validate.lvm_vg.PhysicalVolume */
        /* If we already know a PV is being created there, we know it's fine */
        if(seen_pvs.find(vg->pv()) == seen_pvs.end()) {
            /* Okay, let's see if a PV already exists there... */
            if(opts.test(InstallEnvironment)) {
#ifdef HAS_INSTALL_ENV
                if(!vg->test_pv()) {
                    failures++;
                    output_error(vg->where(), "lvm_vg: a physical volume does "
                                              "not exist on " + vg->pv());
                }
#endif /* HAS_INSTALL_ENV */
            } else {
                /* We can't tell if we aren't running on the target. */
                output_warning(vg->where(), "lvm_vg: please ensure an LVM "
                               "physical volume already exists at " + vg->pv());
            }
        }
    }

    /* REQ: Runner.Validate.lvm_lv */
    for(auto &lv : internal->lvm_lvs) {
        const std::string lvpath(lv->vg() + "/" + lv->name());
        VALIDATE_OR_SKIP(lv)

        /* REQ: Runner.Validate.lvm_lv.Name */
        if(seen_lvs.find(lvpath) != seen_lvs.end()) {
            failures++;
            output_error(lv->where(), "lvm_lv: a volume with the name " +
                         lv->name() + " already exists on the volume group " +
                         lv->vg());
        } else {
            seen_lvs.insert(lvpath);
        }

        /* REQ: Runner.Validate.lvm_lv.VolumeGroup */
        if(seen_vg_names.find(lv->vg()) == seen_vg_names.end()) {
            /* Let's make sure it still exists, if we are running in the IE */
            if(opts.test(InstallEnvironment)) {
#ifdef HAS_INSTALL_ENV
                if(!fs::exists("/dev/" + lv->vg())) {
                    failures++;
                    output_error(lv->where(), "lvm_lv: volume group " +
                                 lv->vg() + " does not exist");
                }
#endif /* HAS_INSTALL_ENV */
            }
        }
    }

#define CHECK_EXIST_PART_LV(device, key, where) \
    if(!fs::exists(device, ec) &&\
       seen_parts.find(device) == seen_parts.end() &&\
       seen_lvs.find(device.substr(5)) == seen_lvs.end()) {\
        failures++;\
        output_error(where, std::string(key) + ": device " + device +\
                     " does not exist");\
    }

    /* REQ: Runner.Validate.encrypt */
    for(auto &crypt : internal->luks) {
        VALIDATE_OR_SKIP(crypt)

        /* REQ: Runner.Validate.encrypt.Unique */
        if(seen_luks.find(crypt->device()) != seen_luks.end()) {
            failures++;
            output_error(crypt->where(), "encrypt: encryption already enabled",
                         crypt->device());
        } else {
            seen_luks.insert(crypt->device());
        }

        /* REQ: Runner.Validate.encrypt.Block */
        if(opts.test(InstallEnvironment)) {
#ifdef HAS_INSTALL_ENV
            CHECK_EXIST_PART_LV(crypt->device(), "encrypt", crypt->where())
#endif /* HAS_INSTALL_ENV */
        }
    }

    /* REQ: Runner.Validate.fs */
    for(auto &fs : internal->fses) {
        VALIDATE_OR_SKIP(fs)

        /* REQ: Runner.Validate.fs.Unique */
        if(seen_fses.find(fs->device()) != seen_fses.end()) {
            failures++;
            output_error(fs->where(), "fs: a filesystem is already scheduled "
                         "to be created on " + fs->device());
        }
        seen_fses.insert(fs->device());

        /* REQ: Runner.Validate.fs.Block */
        if(opts.test(InstallEnvironment)) {
#ifdef HAS_INSTALL_ENV
            CHECK_EXIST_PART_LV(fs->device(), "fs", fs->where())
#endif /* HAS_INSTALL_ENV */
        }
    }

    /* REQ: Runner.Validate.mount */
    for(auto &mount : internal->mounts) {
        VALIDATE_OR_SKIP(mount)

        /* REQ: Runner.Validate.mount.Unique */
        if(seen_mounts.find(mount->mountpoint()) != seen_mounts.end()) {
            failures++;
            output_error(mount->where(), "mount: mountpoint " +
                         mount->mountpoint() + " has already been specified; " +
                         mount->device() + " is a duplicate");
        } else {
            seen_mounts.insert(mount->mountpoint());
        }

        /* REQ: Runner.Validate.mount.Block */
        if(opts.test(InstallEnvironment)) {
#ifdef HAS_INSTALL_ENV
            CHECK_EXIST_PART_LV(mount->device(), "mount", mount->where())
#endif /* HAS_INSTALL_ENV */
        }
    }

#undef CHECK_EXIST_PART_LV

    /* REQ: Runner.Validate.mount.Root */
    if(seen_mounts.find("/") == seen_mounts.end() && !opts.test(ImageOnly)) {
        failures++;
        output_error("installfile:0", "mount: no root mount specified");
    }

#undef VALIDATE_OR_SKIP

    output_log("validator", "0", "installfile",
               to_string(failures) + " failure(s).", "");
    return (failures == 0);
}

}
