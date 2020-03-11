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
#include <set>
#include <string>
#ifdef HAS_INSTALL_ENV
#   include <parted/parted.h>
#endif /* HAS_INSTALL_ENV */

#include "script.hh"
#include "script_i.hh"

#include "util.hh"
#include "util/filesystem.hh"

namespace Horizon {

static bool icon_dir_created = false;

void maybe_create_icon_dir(ScriptOptions opts, const std::string &target) {
    if(icon_dir_created) return;
    icon_dir_created = true;

    const std::string icon_dir(target + "/var/lib/AccountsService/icons");

    if(opts.test(Simulate)) {
        std::cout << "mkdir -p " << icon_dir << std::endl
                  << "chown root:root " << icon_dir << std::endl
                  << "chmod 775 " << icon_dir << std::endl;
        return;
    }
#ifdef HAS_INSTALL_ENV
    else {
        error_code ec;
        if(!fs::exists(icon_dir, ec)) {
            fs::create_directories(icon_dir, ec);
            if(ec) {
                output_error("internal", "couldn't create icon dir",
                             ec.message());
            }
        }
    }
#endif  /* HAS_INSTALL_ENV */
}

bool Script::execute() const {
    bool success;
    error_code ec;
    std::set<std::string> ifaces;
    const std::string targ_etc(targetDirectory() + "/etc");

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
#define EXECUTE_OR_FAIL(phase, obj) \
    if(!obj->execute()) {\
        EXECUTE_FAILURE(phase);\
        return false;\
    }

    /**************** DISK SETUP ****************/
    output_step_start("disk");
#ifdef HAS_INSTALL_ENV
    if(opts.test(InstallEnvironment)) ped_device_probe_all();
#endif /* HAS_INSTALL_ENV */
    /* REQ: Runner.Execute.diskid */
    for(auto &diskid : internal->diskids) {
        EXECUTE_OR_FAIL("diskid", diskid)
    }

    /* REQ: Runner.Execute.disklabel */
    for(auto &label : internal->disklabels) {
        EXECUTE_OR_FAIL("disklabel", label)
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
        EXECUTE_OR_FAIL("partition", part)
    }

    /* encrypt PVs */

    /* REQ: Runner.Execute.lvm_pv */
    for(auto &pv : internal->lvm_pvs) {
        EXECUTE_OR_FAIL("lvm_pv", pv)
    }

    /* REQ: Runner.Execute.lvm_vg */
    for(auto &vg : internal->lvm_vgs) {
        EXECUTE_OR_FAIL("lvm_vg", vg)
    }

    /* REQ: Runner.Execute.lvm_lv */
    for(auto &lv : internal->lvm_lvs) {
        EXECUTE_OR_FAIL("lvm_lv", lv)
    }

    /* encrypt */

    /* REQ: Runner.Execute.fs */
    for(auto &fs : internal->fses) {
        EXECUTE_OR_FAIL("fs", fs)
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
        EXECUTE_OR_FAIL("mount", mount)
    }
#ifdef HAS_INSTALL_ENV
    if(opts.test(InstallEnvironment)) ped_device_free_all();
#endif /* HAS_INSTALL_ENV */
    output_step_end("disk");

    /**************** PRE PACKAGE METADATA ****************/
    output_step_start("pre-metadata");

    /* REQ: Runner.Execute.hostname */
    EXECUTE_OR_FAIL("hostname", internal->hostname)

    /* REQ: Runner.Execute.repository */
    if(opts.test(Simulate)) {
        std::cout << "mkdir -p " << targ_etc << "/apk" << std::endl;
    }
#ifdef HAS_INSTALL_ENV
    else {
        if(!fs::exists(targ_etc + "/apk", ec)) {
            fs::create_directory(targ_etc + "/apk", ec);
            if(ec) {
                output_error("internal", "failed to initialise APK");
                EXECUTE_FAILURE("pre-metadata");
                return false;
            }
        }
    }
#endif /* HAS_INSTALL_ENV */
    for(auto &repo : internal->repos) {
        EXECUTE_OR_FAIL("repository", repo)
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
            if(!ssid->execute()) {
                EXECUTE_FAILURE("ssid");
                /* "Soft" error.  Not fatal. */
            }
        }

        if(opts.test(Simulate)) {
            std::ifstream wpai("/tmp/horizon/wpa_supplicant.conf");
            if(wpai) {
                std::cout << "cat >" << targetDirectory()
                          << "/etc/wpa_supplicant/wpa_supplicant.conf "
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
            const std::string wpa_dir(targ_etc + "/wpa_supplicant");
            if(!fs::exists(wpa_dir, ec)) {
                fs::create_directory(wpa_dir, ec);
            }
            fs::copy_file("/tmp/horizon/wpa_supplicant.conf",
                          wpa_dir + "/wpa_supplicant.conf",
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
            if(!addr->execute()) {
                EXECUTE_FAILURE("netaddress");
                /* "Soft" error.  Not fatal. */
            } else {
                ifaces.insert(addr->iface());
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
            std::cout << "mkdir -p " << targ_etc << "/conf.d" << std::endl;
            std::cout << "cat >>/" << targ_etc << "/conf.d/net <<- NETCONF_EOF"
                      << std::endl << conf.str() << std::endl
                      << "NETCONF_EOF" << std::endl;
        }
#ifdef HAS_INSTALL_ENV
        else {
            if(!fs::exists(targ_etc + "/conf.d")) {
                fs::create_directory(targ_etc + "/conf.d", ec);
                if(ec) {
                    output_error("internal", "could not create /etc/conf.d "
                                 "directory", ec.message());
                }
            }
            std::ofstream conf_file(targ_etc + "/conf.d/net",
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
            EXECUTE_OR_FAIL("nameserver", ns)
        }

        /* REQ: Runner.Execute.nameserver.FQDN */
        const std::string &hostname(internal->hostname->value());
        const std::string::size_type dot = hostname.find_first_of('.');
        if(dot != std::string::npos && hostname.size() > dot + 1) {
            const std::string domain(hostname.substr(dot + 1));
            if(opts.test(Simulate)) {
                std::cout << "printf 'domain " << domain << "\\"
                          << "n >>" << targ_etc << "/resolv.conf" << std::endl;
            }
#ifdef HAS_INSTALL_ENV
            else {
                std::ofstream resolvconf(targ_etc + "/resolv.conf",
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
                std::cout << "mv " << targ_etc << "/resolv.conf "
                          << targ_etc << "/resolv.conf.head" << std::endl;
            }
#ifdef HAS_INSTALL_ENV
            else {
                if(fs::exists(targ_etc + "/resolv.conf", ec)) {
                    fs::rename(targ_etc + "/resolv.conf",
                               targ_etc + "/resolv.conf.head", ec);
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

    EXECUTE_OR_FAIL("network", internal->network)

    if(internal->network->test()) {
        bool do_wpa = !internal->ssids.empty();

        if(opts.test(Simulate)) {
            if(do_wpa) {
                std::cout << "cp " << targ_etc
                          << "/wpa_supplicant/wpa_supplicant.conf "
                          << "/etc/wpa_supplicant/wpa_supplicant.conf"
                          << std::endl;
            }
            std::cout << "cp " << targ_etc << "/conf.d/net /etc/conf.d/net"
                      << std::endl;
            if(!internal->nses.empty()) {
                std::cout << "cp " << targ_etc << "/resolv.conf* /etc/"
                          << std::endl;
            }
        }
#ifdef HAS_INSTALL_ENV
        else {
            if(do_wpa) {
                fs::copy_file(targ_etc + "/wpa_supplicant/wpa_supplicant.conf",
                          "/etc/wpa_supplicant/wpa_supplicant.conf",
                          fs_overwrite, ec);
                if(ec) {
                    output_error("internal", "cannot use wireless configuration "
                                 "during installation", ec.message());
                    EXECUTE_FAILURE("network");
                }
            }
            fs::copy_file(targ_etc + "/conf.d/net", "/etc/conf.d/net",
                          fs_overwrite, ec);
            if(ec) {
                output_error("internal", "cannot use networking configuration "
                             "during installation", ec.message());
                EXECUTE_FAILURE("network");
                return false;
            }
            for(auto &iface : ifaces) {
                fs::create_symlink("/etc/init.d/net.lo",
                                   "/etc/init.d/net." + iface, ec);
                if(ec) {
                    output_error("internal", "could not use networking on "
                                 + iface, ec.message());
                    EXECUTE_FAILURE("network");
                } else {
                    run_command("service", {"net." + iface, "start"});
                }
            }
            if(!internal->nses.empty()) {
                if(dhcp) {
                    fs::copy_file(targ_etc + "/resolv.conf.head",
                                  "/etc/resolv.conf.head", ec);
                } else {
                    fs::copy_file(targ_etc + "/resolv.conf",
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
        EXECUTE_OR_FAIL("signingkey", key)
    }

    /* REQ: Runner.Execute.pkginstall.APKDB */
    output_info("internal", "initialising APK");
    if(opts.test(Simulate)) {
        std::cout << "/sbin/apk --root " << targetDirectory() << " --initdb add"
                  << std::endl;
    }
#ifdef HAS_INSTALL_ENV
    else {
        if(run_command("/sbin/apk",
                       {"--root", targetDirectory(), "--initdb", "add"}) != 0) {
            EXECUTE_FAILURE("pkginstall");
            return false;
        }
    }
#endif  /* HAS_INSTALL_ENV */

    /* REQ: Runner.Execute.pkginstall.arch */
    if(internal->arch) {
        EXECUTE_OR_FAIL("arch", internal->arch)
    }

    /* REQ: Runner.Execute.pkginstall */
    output_info("internal", "installing packages to target");
    if(opts.test(Simulate)) {
        std::ostringstream pkg_list;
        for(auto &pkg : this->internal->packages) {
            pkg_list << pkg << " ";
        }

        std::cout << "apk --root " << targetDirectory() << " update"
                  << std::endl;
        std::cout << "apk --root " << targetDirectory() << " add "
                  << pkg_list.str() << std::endl;
    }
#ifdef HAS_INSTALL_ENV
    else {
        if(run_command("/sbin/apk",
                       {"--root", targetDirectory(), "update"}) != 0) {
            EXECUTE_FAILURE("pkginstall");
            return false;
        }

        std::vector<std::string> params = {"--root", targetDirectory(), "add"};
        std::copy(this->internal->packages.begin(),
                  this->internal->packages.end(),
                  params.end());

        if(run_command("/sbin/apk", params) != 0) {
            EXECUTE_FAILURE("pkginstall");
            return false;
        }
    }
#endif  /* HAS_INSTALL_ENV */

    output_step_end("pkgdb");

    /**************** POST PACKAGE METADATA ****************/
    output_step_start("post-metadata");

    if(internal->packages.find("netifrc") != internal->packages.end() &&
       !internal->addresses.empty()) {
        /* REQ: Runner.Execute.netaddress.OpenRC */
        if(opts.test(Simulate)) {
            for(auto &iface : ifaces) {
                std::cout << "ln -s /etc/init.d/net.lo " << targ_etc
                          << "/init.d/net." << iface << std::endl;
                std::cout << "ln -s /etc/init.d/net." << iface
                          << " " << targ_etc << "/runlevels/default/net."
                          << iface << std::endl;
            }
        }
#ifdef HAS_INSTALL_ENV
        else {
            for(auto &iface : ifaces) {
                fs::create_symlink("/etc/init.d/net.lo",
                                   targ_etc + "/init.d/net." + iface, ec);
                if(ec) {
                    output_error("internal", "could not set up networking on "
                                 + iface, ec.message());
                } else {
                    fs::create_symlink("/etc/init.d/net." + iface,
                                       targ_etc + "/runlevels/default/net." +
                                       iface, ec);
                    if(ec) {
                        output_error("internal", "could not auto-start "
                                     "networking on " + iface, ec.message());
                    }
                }
            }
        }
#endif  /* HAS_INSTALL_ENV */
    }

    EXECUTE_OR_FAIL("rootpw", internal->rootpw)

    if(internal->lang) {
        EXECUTE_OR_FAIL("language", internal->lang)
    }

    if(internal->keymap) {
        EXECUTE_OR_FAIL("keymap", internal->keymap)
    }

    for(auto &acct : internal->accounts) {
        output_info("internal", "setting up user account " + acct.first);

        EXECUTE_OR_FAIL("username", acct.second->name)
        if(acct.second->alias) {
            EXECUTE_OR_FAIL("useralias", acct.second->alias)
        }
        if(acct.second->passphrase) {
            EXECUTE_OR_FAIL("userpw", acct.second->passphrase)
        }
        if(!acct.second->groups.empty()) {
            for(auto &grp : acct.second->groups) {
                EXECUTE_OR_FAIL("usergroups", grp)
            }
        }
        if(acct.second->icon) {
            maybe_create_icon_dir(opts, targetDirectory());
            EXECUTE_OR_FAIL("usericon", acct.second->icon)
        }
    }

    EXECUTE_OR_FAIL("timezone", internal->tzone)

    output_step_end("post-metadata");
    return true;
}

}
