/*
 * iso.cc - Implementation of the CD image Horizon Image Creation backend
 * image, the image processing utilities for
 * Project Horizon
 *
 * Copyright (c) 2020 Adélie Linux and contributors.  All rights reserved.
 * This code is licensed under the AGPL 3.0 license, as noted in the
 * LICENSE-code file in the root directory of this repository.
 *
 * SPDX-License-Identifier: AGPL-3.0-only
 */

#include <cstdlib>      /* getenv */
#include <cstring>      /* strlen, strtok */
#include <fstream>      /* ifstream, ofstream */
#include <boost/algorithm/string.hpp>

#include <sys/mount.h>

#include "basic.hh"
#include "hscript/util.hh"
#include "util/filesystem.hh"
#include "util/output.hh"

using namespace boost::algorithm;

const std::vector<std::string> data_dirs() {
    std::vector<std::string> dirs;

    char *home = std::getenv("XDG_DATA_HOME");
    if(home != nullptr && std::strlen(home) > 0) {
        dirs.push_back(home);
    } else {
        home = std::getenv("HOME");
        if(home != nullptr && std::strlen(home) > 0) {
            dirs.push_back(std::string(home) + "/.local/share");
        } else {
            home = std::getenv("APPDATA");
            if(home != nullptr) {
                dirs.push_back(home);
            } else {
                /* Give up. */
            }
        }
    }

    const char *sys_c = std::getenv("XDG_DATA_DIRS");
    if(sys_c == nullptr || std::strlen(sys_c) == 0) {
        sys_c = "/usr/local/share:/usr/share";
    }

    const std::string sys{sys_c};
    std::vector<std::string> temp;
    boost::split(temp, sys, is_any_of(":"));
    std::move(temp.begin(), temp.end(), std::back_inserter(dirs));

    return dirs;
}

const fs::path find_data_file(std::string name) {
    error_code ec;
    for(const auto &p : data_dirs()) {
        fs::path src = fs::path(p).append("horizon").append("iso").append(name);

        if(fs::exists(src, ec)) {
            return src;
        }
    }
    return fs::path();
}

bool copy_volume_icon_to(fs::path ir_dir) {
    error_code ec;

    const fs::path dest = ir_dir.append("cdroot").append("VolumeIcon.icns");
    const fs::path src = find_data_file("VolumeIcon.icns");

    /* No volume icon. */
    if(!src.has_filename()) return false;

    fs::copy(src, dest, ec);
    if(ec && ec.value() != EEXIST) {
        output_error("CD backend", "could not copy volume icon", ec.message());
        return false;
    }
    return true;
}

bool write_etc_mtab_to(fs::path target) {
    std::ofstream mtab(target.append("etc/conf.d/mtab"));
    if(!mtab) {
        output_error("CD backend", "failed to open mtab configuration");
        return false;
    }
    mtab << "mtab_is_file=no" << std::endl;
    if(mtab.fail() || mtab.bad()) {
        output_error("CD backend", "failed to write mtab configuration");
        return false;
    }
    mtab.flush();
    mtab.close();
    return true;
}

bool write_fstab_to(fs::path target) {
    std::ofstream fstab{target.append("etc/fstab")};
    if(!fstab) {
        output_error("CD backend", "failed to open fstab");
        return false;
    }

    fstab << "# Welcome to Adélie Linux." << std::endl
          << "# This fstab(5) is for the live media only.  "
          << "Do not edit or use for your installation." << std::endl
          << std::endl
          << "tmpfs   /tmp            tmpfs   defaults        0       1"
          << std::endl
          << "proc    /proc           proc    defaults        0       1"
          << std::endl;

    if(fstab.fail() || fstab.bad()) {
        output_error("CD backend", "failed to write fstab");
        return false;
    }
    fstab.flush();
    fstab.close();
    return true;
}

bool write_etc_issue_to(fs::path target) {
    error_code ec;
    const fs::path dest{target.append("etc/issue")};

    const fs::path src{find_data_file("issue")};
    if(src.has_filename()) {
        fs::copy(src, dest, ec);
        return !ec;
    }

    /* We don't have a file, so write out our default. */
    std::ofstream issue(dest);
    if(!issue) {
        output_error("CD backend", "failed to open issue file");
        return false;
    }

    issue << "Welcome to Adélie Linux!" << std::endl
          << "You may log in as 'root' to install, or 'live' to play around."
          << std::endl
          << "The default root password is 'live' (without quotes)."
          << std::endl << std::endl << "Have fun." << std::endl;

    if(issue.fail() || issue.bad()) {
        output_error("CD backend", "failed to write issue file");
        return false;
    }
    issue.flush();
    issue.close();
    return true;
}

namespace Horizon {
namespace Image {

class CDBackend : public BasicBackend {
public:
    enum CDError {
        COMMAND_MISSING = 1,
        FS_ERROR,
        COMMAND_ERROR
    };

    explicit CDBackend(const std::string &ir, const std::string &out,
                       const std::map<std::string, std::string> &opts)
        : BasicBackend(ir, out, opts) {};

    int prepare() override {
        error_code ec;

        output_info("CD backend", "probing SquashFS tools version...");
        if(run_command("mksquashfs", {"-version"}) != 0) {
            output_error("CD backend", "SquashFS tools are not present");
            return COMMAND_MISSING;
        }

        /* REQ: ISO.1 */
        if(fs::exists(this->ir_dir, ec)) {
            output_info("CD backend", "removing old IR tree", this->ir_dir);
            /* try to umount first, just in case
             * We don't care if the call fails.
             */
            for(const std::string &mount : {"dev", "proc", "sys"}) {
                const std::string path = this->ir_dir + "/target/" + mount;
                ::umount(path.c_str());
            }
            fs::remove_all(this->ir_dir, ec);
            if(ec) {
                output_warning("CD backend", "could not remove IR tree",
                               ec.message());
                /* we can _try_ to proceed anyway... */
            }
        }

        output_info("CD backend", "creating directory tree");

        /* REQ: ISO.2 */
        fs::create_directory(this->ir_dir, ec);
        if(ec && ec.value() != EEXIST) {
            output_error("CD backend", "could not create IR directory",
                         ec.message());
            return FS_ERROR;
        }

        /* REQ: ISO.2 */
        fs::create_directory(this->ir_dir + "/cdroot", ec);
        if(ec && ec.value() != EEXIST) {
            output_error("CD backend", "could not create ISO directory",
                         ec.message());
            return FS_ERROR;
        }

        /* REQ: ISO.2 */
        fs::create_directory(this->ir_dir + "/target", ec);
        if(ec && ec.value() != EEXIST) {
            output_error("CD backend", "could not create target directory",
                         ec.message());
            return FS_ERROR;
        }

        /* REQ: ISO.4 */
        fs::create_directories(this->ir_dir + "/target/etc/default", ec);
        if(ec && ec.value() != EEXIST) {
            output_error("CD backend", "could not create target config dir",
                         ec.message());
            return FS_ERROR;
        }

        /* REQ: ISO.4 */
        output_info("CD backend", "configuring boot loader");
        std::ofstream grub(this->ir_dir + "/target/etc/default/grub");
        grub << "ADELIE_MANUAL_CONFIG=1" << std::endl;
        if(grub.fail() || grub.bad()) {
            output_error("CD backend", "failed to configure GRUB");
            return FS_ERROR;
        }
        grub.close();

        return 0;
    }

    int create() override {
        error_code ec;
        std::string my_arch;
        std::ifstream archstream(this->ir_dir + "/target/etc/apk/arch");
        const std::string target = this->ir_dir + "/target";
        const std::string cdpath = this->ir_dir + "/cdroot";
        archstream >> my_arch;

        fs::current_path(this->ir_dir);

        /* REQ: ISO.7 */
        output_info("CD backend", "creating live environment directories");
        fs::create_directory(target + "/target", ec);
        if(ec && ec.value() != EEXIST) {
            output_error("CD backend", "could not create directory",
                         ec.message());
        }
        fs::create_directories(target + "/media/live", ec);
        if(ec && ec.value() != EEXIST) {
            output_error("CD backend", "could not create directory",
                         ec.message());
        }

        /* REQ: ISO.9 */
        output_info("CD backend", "configuring mtab");
        write_etc_mtab_to(target);

        /* REQ: ISO.10 */
        output_info("CD backend", "enabling required services");
        const std::string targetsi = target + "/etc/runlevels/sysinit/";
        for(const std::string &svc : {"udev", "udev-trigger", "lvmetad"}) {
            fs::create_symlink("/etc/init.d/" + svc, targetsi + svc, ec);
            if(ec && ec.value() != EEXIST) {
                output_error("CD backend", "could not enable service " + svc,
                             ec.message());
                return FS_ERROR;
            }
        }

        /* REQ: ISO.12 */
        output_info("CD backend", "creating live environment /etc/fstab");
        if(!write_fstab_to(target)) return FS_ERROR;

        /* REQ: ISO.13 */
        output_info("CD backend", "setting root shell");
        run_command("sed", {"-i", "s#/root:/bin/sh$#/root:/bin/zsh#",
                            target + "/etc/passwd"});

        /* REQ: ISO.15 */
        output_info("CD backend", "configuring login services");
        run_command("sed", {"-i", "s/pam_unix.so$/pam_unix.so nullok_secure/",
                            target + "/etc/pam.d/base-auth"});
        run_command("chroot", {target, "/usr/bin/passwd", "-d", "live"});

        /* REQ: ISO.19 */
        output_info("CD backend", "creating live /etc/issue");
        if(opts.find("issue-path") != opts.end() &&
                fs::exists(opts.at("issue-path"))) {
            fs::path dest = fs::path(cdpath).append("etc/issue");
            fs::copy(opts.at("issue-path"), dest, ec);
            if(ec) output_error("CD backend", "could not copy /etc/issue",
                                ec.message());
        } else if(!write_etc_issue_to(target)) return FS_ERROR;

        /* REQ: ISO.22 */
        output_info("CD backend", "generating file list");
        {
            std::ofstream exclude(this->ir_dir + "/exclude.list");
            exclude << "dev/*" << std::endl
                    << "proc/*" << std::endl
                    << "sys/*" << std::endl;
            if(exclude.fail() || exclude.bad()) {
                output_error("CD backend", "failed to write exclusion list");
                return FS_ERROR;
            }
            exclude.flush();
            exclude.close();
        }

        /* REQ: ISO.22 */
        output_info("CD backend", "creating SquashFS");
        const std::string squashpath = cdpath + "/" + my_arch + ".squashfs";
        if(run_command("mksquashfs", {target, squashpath, "-noappend",
                                      "-wildcards", "-ef",
                                      this->ir_dir + "/exclude.list"}) != 0) {
            output_error("CD backend", "failed to create SquashFS");
            return COMMAND_ERROR;
        }

        /* REQ: ISO.21 */
        if(opts.find("icon-path") != opts.end() &&
                fs::exists(opts.at("icon-path"))) {
            fs::path dest = fs::path(cdpath).append("VolumeIcon.icns");
            fs::copy(opts.at("icon-path"), dest, ec);
            if(ec) output_error("CD backend", "could not copy volume icon",
                                ec.message());
        } else if(!copy_volume_icon_to(this->ir_dir)) {
            output_warning("CD backend", "No volume icon could be found.");
        }

        /* REQ: ISO.23 */
        output_info("CD backend", "creating initrd");
        std::string cdinit_path{""};
        for(const auto &path : data_dirs()) {
            fs::path candidate{fs::path{path}.append("horizon").append("iso")
                               .append("cdinits")};
            if(fs::exists(candidate, ec)) {
                cdinit_path = candidate;
            }
        }
#include "initrd.sh.cpp"
        if(run_command("/bin/sh", {"-ec", initrd, "mkinitrd", my_arch, target,
                                   cdpath, cdinit_path}) != 0) {
            output_error("CD backend", "failed to create initrd");
            return COMMAND_ERROR;
        }

        /* REQ: ISO.24 */
        std::string postscript;
        if(opts.find("post-script") != opts.end() &&
                fs::exists(opts.at("post-script"))) {
            postscript = opts.at("post-script");
        } else {
            for(const auto &path : data_dirs()) {
                fs::path candidate{fs::path{path}.append("horizon")
                            .append("iso").append("post-" + my_arch + ".sh")};
                if(fs::exists(candidate, ec)) {
                    postscript = candidate;
                    break;
                }
            }
        }
        if(postscript.length() > 0) {
            output_info("CD backend", "running architecture-specific script",
                        postscript);
            if(run_command("/bin/sh", {"-e", postscript}) != 0) {
                output_error("CD backend", "architecture-specific script failed");
                return COMMAND_ERROR;
            }
        }

        /* REQ: ISO.25 */
        output_info("CD backend", "installing kernel");
        for(const auto &candidate : fs::directory_iterator(target + "/boot")) {
            auto name = candidate.path().filename().string();
            if(name.length() > 6 && name.substr(0, 6) == "vmlinu") {
                fs::copy(candidate.path(), cdpath + "/kernel-" + my_arch, ec);
                if(ec) {
                    output_error("CD backend", "failed to copy kernel",
                                 ec.message());
                    return FS_ERROR;
                }
                fs::remove(candidate.path(), ec);
                break;
            }
        }

        /* REQ: ISO.26 */
        output_info("CD backend", "creating ISO");
        std::vector<std::string> iso_args = {"-as", "mkisofs", "-o", out_path,
                                             "-joliet", "-rational-rock", "-V",
                                             "Adelie "+my_arch};
        std::vector<std::string> arch_args;
        std::string raw_arch;
        {
            const fs::path param_file = find_data_file("iso-params-" + my_arch);
            if(param_file.has_filename()) {
                std::ifstream params{param_file};
                if(!params) {
                    output_warning("CD backend", "couldn't read ISO params");
                } else {
                    std::getline(params, raw_arch);
                }
            }
        }
        if(raw_arch.length() > 0) {
            boost::split(arch_args, raw_arch, is_any_of(" "));
            std::move(arch_args.begin(), arch_args.end(),
                      std::back_inserter(iso_args));
        }
        if(opts.find("iso-params") != opts.end()) {
            boost::split(arch_args, opts.at("iso-params"), is_any_of(" "));
            std::move(arch_args.begin(), arch_args.end(),
                      std::back_inserter(iso_args));
        }
        iso_args.push_back(cdpath);
        return run_command("xorriso", iso_args);
    }

    int finalise() override {
        output_info("CD backend", "Live image created successfully", out_path);
        return 0;
    }
};

__attribute__((constructor(400)))
void register_cd_backend() {
    BackendManager::register_backend(
    {"iso", "Create a CD image (.iso)",
        [](const std::string &ir_dir, const std::string &out_path,
           const std::map<std::string, std::string> &opts) {
            return new CDBackend(ir_dir, out_path, opts);
        }
    });
}

}
}
