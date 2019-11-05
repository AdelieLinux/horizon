/*
 * script_e.cc - Implementation of Script::execute
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
#ifdef HAS_INSTALL_ENV
#   include <parted/parted.h>
#endif /* HAS_INSTALL_ENV */

#include "script.hh"
#include "script_i.hh"

#include "util/filesystem.hh"

namespace Horizon {

bool Script::execute() const {
    bool success;
    error_code ec;

    /* assume create_directory will give us the error if removal fails */
    if(fs::exists("/tmp/horizon", ec)) {
        fs::remove_all("/tmp/horizon", ec);
    }

    if(!fs::create_directory("/tmp/horizon", ec)) {
        output_error("internal", "could not create temporary directory",
                     ec.message());
        return false;
    }

    /* REQ: Runner.Execute.Verify */
    output_step_start("validate");
    success = this->validate();
    output_step_end("validate");
    if(!success) {
        /* REQ: Runner.Execute.Verify.Failure */
        output_error("validator", "The HorizonScript failed validation",
                     "Check the output from the validator.");
        return false;
    }

#define EXECUTE_FAILURE(phase) \
    output_error(phase, "The HorizonScript failed to execute",\
                 "Check the log file for more details.")

    /**************** DISK SETUP ****************/
    output_step_start("disk");
#ifdef HAS_INSTALL_ENV
    if(opts.test(InstallEnvironment)) ped_device_probe_all();
#endif /* HAS_INSTALL_ENV */
    /* REQ: Runner.Execute.diskid */
    for(auto &diskid : internal->diskids) {
        if(!diskid->execute(opts)) {
            EXECUTE_FAILURE("diskid");
            return false;
        }
    }

    /* REQ: Runner.Execute.disklabel */
    for(auto &label : internal->disklabels) {
        if(!label->execute(opts)) {
            EXECUTE_FAILURE("disklabel");
            return false;
        }
    }

    /* REQ: Runner.Execute.partition */
    /* Ensure partitions are created in on-disk order. */
    std::sort(internal->partitions.begin(), internal->partitions.end(),
              [](std::unique_ptr<Partition> const &e1,
                 std::unique_ptr<Partition> const &e2) {
        return (e1->device() + "p" + std::to_string(e1->partno())) <
               (e2->device() + "p" + std::to_string(e2->partno()));
    });
    for(auto &part : internal->partitions) {
        if(!part->execute(opts)) {
            EXECUTE_FAILURE("partition");
            return false;
        }
    }

    /* encrypt PVs */

    /* REQ: Runner.Execute.lvm_pv */
    for(auto &pv : internal->lvm_pvs) {
        if(!pv->execute(opts)) {
            EXECUTE_FAILURE("lvm_pv");
            return false;
        }
    }

    /* REQ: Runner.Execute.lvm_vg */
    for(auto &vg : internal->lvm_vgs) {
        if(!vg->execute(opts)) {
            EXECUTE_FAILURE("lvm_vg");
            return false;
        }
    }

    /* REQ: Runner.Execute.lvm_lv */
    for(auto &lv : internal->lvm_lvs) {
        if(!lv->execute(opts)) {
            EXECUTE_FAILURE("lvm_lv");
            return false;
        }
    }

    /* encrypt */

    /* REQ: Runner.Execute.fs */
    for(auto &fs : internal->fses) {
        if(!fs->execute(opts)) {
            EXECUTE_FAILURE("fs");
            return false;
        }
    }

    /* REQ: Runner.Execute.mount */
    /* Sort by mountpoint.
     * This ensures that any subdirectory mounts come after their parent. */
    std::sort(internal->mounts.begin(), internal->mounts.end(),
              [](std::unique_ptr<Mount> const &e1,
                 std::unique_ptr<Mount> const &e2) {
        return e1->mountpoint() < e2->mountpoint();
    });
    for(auto &mount : internal->mounts) {
        if(!mount->execute(opts)) {
            EXECUTE_FAILURE("mount");
            return false;
        }
    }
#ifdef HAS_INSTALL_ENV
    if(opts.test(InstallEnvironment)) ped_device_free_all();
#endif /* HAS_INSTALL_ENV */
    output_step_end("disk");

    /**************** PRE PACKAGE METADATA ****************/
    output_step_start("pre-metadata");

    /* REQ: Runner.Execute.hostname */
    if(!internal->hostname->execute(opts)) {
        EXECUTE_FAILURE("hostname");
        return false;
    }

    /* REQ: Runner.Execute.repository */
    if(opts.test(Simulate)) {
        std::cout << "mkdir -p /target/etc/apk" << std::endl;
    }
#ifdef HAS_INSTALL_ENV
    else {
        if(!fs::exists("/target/etc/apk", ec)) {
            fs::create_directory("/target/etc/apk", ec);
            if(ec) {
                output_error("internal", "failed to initialise APK");
                EXECUTE_FAILURE("pre-metadata");
                return false;
            }
        }
    }
#endif /* HAS_INSTALL_ENV */
    for(auto &repo : internal->repos) {
        if(!repo->execute(opts)) {
            EXECUTE_FAILURE("repository");
            return false;
        }
    }

#ifdef NON_LIBRE_FIRMWARE
    /* REQ: Runner.Execute.firmware */
    if(internal->firmware && internal->firmware->test()) {
        internal->packages.insert("linux-firmware");
    }
#endif
    output_step_end("pre-metadata");

    /**************** NETWORK ****************/
    output_step_start("net");

    if(!this->internal->ssids.empty()) {
        std::ofstream wpao("/tmp/horizon/wpa_supplicant.conf",
                          std::ios_base::trunc);
        if(wpao) {
            wpao << "# Enable the control interface for wpa_cli and wpa_gui"
                << std::endl
                << "ctrl_interface=/var/run/wpa_supplicant" << std::endl
                << "ctrl_interface_group=wheel" << std::endl
                << "update_config=1" << std::endl;
            wpao.close();
        } else {
            output_error("internal",
                         "cannot write wireless networking configuration");
        }

        for(auto &ssid : internal->ssids) {
            if(!ssid->execute(opts)) {
                EXECUTE_FAILURE("ssid");
                /* "Soft" error.  Not fatal. */
            }
        }

        if(opts.test(Simulate)) {
            std::ifstream wpai("/tmp/horizon/wpa_supplicant.conf");
            if(wpai) {
                std::cout << "cat >/target/etc/wpa_supplicant/wpa_supplicant.conf "
                          << "<<- WPA_EOF" << std::endl
                          << wpai.rdbuf() << std::endl
                          << "WPA_EOF" << std::endl;
            } else {
                output_error("internal",
                             "cannot read wireless networking configuration");
            }
        }
#ifdef HAS_INSTALL_ENV
        else {
            if(!fs::exists("/target/etc/wpa_supplicant", ec)) {
                fs::create_directory("/target/etc/wpa_supplicant", ec);
            }
            fs::copy_file("/tmp/horizon/wpa_supplicant.conf",
                          "/target/etc/wpa_supplicant/wpa_supplicant.conf",
                          fs_overwrite, ec);
            if(ec) {
                output_error("internal", "cannot save wireless networking "
                             "configuration to target", ec.message());
            }
        }
#endif /* HAS_INSTALL_ENV */
    }

    bool dhcp = false;
    if(!internal->addresses.empty()) {
        fs::path netifrc_dir("/tmp/horizon/netifrc");
        if(!fs::exists(netifrc_dir) &&
           !fs::create_directory(netifrc_dir, ec)) {
            output_error("internal", "cannot create temporary directory",
                         ec.message());
        }

        for(auto &addr : internal->addresses) {
            if(!addr->execute(opts)) {
                EXECUTE_FAILURE("netaddress");
                /* "Soft" error.  Not fatal. */
            }
            if(addr->type() == NetAddress::DHCP) dhcp = true;
        }

        std::ostringstream conf;
        for(auto &&var_dent : fs::directory_iterator(netifrc_dir)) {
            const std::string variable(var_dent.path().filename().string());
            std::ifstream contents(var_dent.path().string());
            if(!contents) {
                output_error("internal", "cannot read network configuration");
                EXECUTE_FAILURE("net-internal");
                continue;
            }
            conf << variable << "=\"";
            if(contents.rdbuf()->in_avail()) conf << contents.rdbuf();
            conf << "\"" << std::endl;
        }

        if(opts.test(Simulate)) {
            std::cout << "mkdir -p /target/etc/conf.d" << std::endl;
            std::cout << "cat >>/target/etc/conf.d/net <<- NETCONF_EOF"
                      << std::endl << conf.str() << std::endl
                      << "NETCONF_EOF" << std::endl;
        }
#ifdef HAS_INSTALL_ENV
        else {
            if(!fs::exists("/target/etc/conf.d")) {
                fs::create_directory("/target/etc/conf.d", ec);
                if(ec) {
                    output_error("internal", "could not create /etc/conf.d "
                                 "directory", ec.message());
                }
            }
            std::ofstream conf_file("/target/etc/conf.d/net",
                                    std::ios_base::app);
            if(!conf_file) {
                output_error("internal", "cannot save network configuration "
                             "to target");
                EXECUTE_FAILURE("net-internal");
            } else {
                conf_file << conf.str();
            }
        }
#endif /* HAS_INSTALL_ENV */
    }

    /* REQ: Runner.Execute.nameserver */
    if(!internal->nses.empty()) {
        for(auto &ns : internal->nses) {
            if(!ns->execute(opts)) {
                EXECUTE_FAILURE("nameserver");
                return false;
            }
        }

        /* REQ: Runner.Execute.nameserver.FQDN */
        const std::string &hostname(internal->hostname->value());
        const std::string::size_type dot = hostname.find_first_of('.');
        if(dot != std::string::npos && hostname.size() > dot + 1) {
            const std::string domain(hostname.substr(dot + 1));
            if(opts.test(Simulate)) {
                std::cout << "printf 'domain " << domain << "\\"
                          << "n >>/target/etc/resolv.conf" << std::endl;
            }
#ifdef HAS_INSTALL_ENV
            else {
                std::ofstream resolvconf("/target/etc/resolv.conf",
                                         std::ios_base::app);
                if(!resolvconf) {
                    output_error("internal", "failed to open resolv.conf");
                    EXECUTE_FAILURE("nameserver");
                    return false;
                }
                resolvconf << "domain " << domain << std::endl;
            }
#endif /* HAS_INSTALL_ENV */
        }

        if(dhcp) {
            if(opts.test(Simulate)) {
                std::cout << "mv /target/etc/resolv.conf "
                          << "/target/etc/resolv.conf.head" << std::endl;
            }
#ifdef HAS_INSTALL_ENV
            else {
                if(fs::exists("/target/etc/resolv.conf", ec)) {
                    fs::rename("/target/etc/resolv.conf",
                               "/target/etc/resolv.conf.head", ec);
                    if(ec) {
                        output_error("internal",
                                     "cannot save nameserver configuration",
                                     ec.message());
                        EXECUTE_FAILURE("nameserver");
                        return false;
                    }
                }
            }
#endif /* HAS_INSTALL_ENV */
        }
    }

    if(!internal->network->execute(opts)) {
        EXECUTE_FAILURE("network");
        return false;
    }

    if(internal->network->test()) {
        bool do_wpa = !internal->ssids.empty();

        if(opts.test(Simulate)) {
            if(do_wpa) {
                std::cout << "cp /target/etc/wpa_supplicant/wpa_supplicant.conf "
                          << "/etc/wpa_supplicant/wpa_supplicant.conf"
                          << std::endl;
            }
            std::cout << "cp /target/etc/conf.d/net /etc/conf.d/net"
                      << std::endl;
            if(!internal->nses.empty()) {
                std::cout << "cp /target/etc/resolv.conf* /etc/" << std::endl;
            }
        }
#ifdef HAS_INSTALL_ENV
        else {
            if(do_wpa) {
                fs::copy_file("/target/etc/wpa_supplicant/wpa_supplicant.conf",
                          "/etc/wpa_supplicant/wpa_supplicant.conf",
                          fs_overwrite, ec);
                if(ec) {
                    output_error("internal", "cannot use wireless configuration "
                                 "during installation", ec.message());
                    EXECUTE_FAILURE("network");
                }
            }
            fs::copy_file("/target/etc/conf.d/net", "/etc/conf.d/net",
                          fs_overwrite, ec);
            if(ec) {
                output_error("internal", "cannot use networking configuration "
                             "during installation", ec.message());
                EXECUTE_FAILURE("network");
                return false;
            }
            if(!internal->nses.empty()) {
                if(dhcp) {
                    fs::copy_file("/target/etc/resolv.conf.head",
                                  "/etc/resolv.conf.head", ec);
                } else {
                    fs::copy_file("/target/etc/resolv.conf",
                                  "/etc/resolv.conf", ec);
                }

                if(ec) {
                    output_error("internal", "cannot use DNS configuration "
                                 "during installation", ec.message());
                    EXECUTE_FAILURE("network");
                    return false;
                }
            }
        }
#endif /* HAS_INSTALL_ENV */
    }
    output_step_end("net");

    /**************** PKGDB ****************/
    output_step_start("pkgdb");

    /* REQ: Runner.Execute.signingkey */
    for(auto &key : internal->repo_keys) {
        if(!key->execute(opts)) {
            EXECUTE_FAILURE("signingkey");
            return false;
        }
    }

    /* REQ: Runner.Execute.pkginstall.APKDB */
    output_info("internal", "initialising APK");
    if(opts.test(Simulate)) {
        std::cout << "apk --root /target --initdb add" << std::endl;
    } else {
        if(system("apk --root /target --initdb add") != 0) {
            EXECUTE_FAILURE("pkginstall");
            return false;
        }
    }

    /* REQ: Runner.Execute.pkginstall */
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
            EXECUTE_FAILURE("pkginstall");
            return false;
        }
        std::string apk_invoc = "apk --root /target add " + pkg_list.str();
        if(system(apk_invoc.c_str()) != 0) {
            EXECUTE_FAILURE("pkginstall");
            return false;
        }
    }

    output_step_end("pkgdb");

    /**************** POST PACKAGE METADATA ****************/
    output_step_start("post-metadata");

    if(!internal->rootpw->execute(opts)) {
        EXECUTE_FAILURE("rootpw");
        return false;
    }

    if(internal->lang && !internal->lang->execute(opts)) {
        EXECUTE_FAILURE("language");
        return false;
    }

    if(internal->keymap && !internal->keymap->execute(opts)) {
        EXECUTE_FAILURE("keymap");
        return false;
    }
    /* UserAccounts */

    if(!internal->tzone->execute(opts)) {
        EXECUTE_FAILURE("timezone");
        return false;
    }

    output_step_end("post-metadata");
    return true;
}

}
