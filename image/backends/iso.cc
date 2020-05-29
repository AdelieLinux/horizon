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

#include <fstream>
#include "basic.hh"
#include "hscript/util.hh"
#include "util/filesystem.hh"
#include "util/output.hh"

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
            fs::remove_all(this->ir_dir, ec);
            if(ec) {
                output_warning("CD backend", "could not remove IR tree",
                               ec.message());
                /* we can _try_ to proceed anyway... */
            }
        }

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
        std::string my_arch;
        std::ifstream archstream(this->ir_dir + "/target/etc/apk/arch");
        const std::string target = this->ir_dir + "/target";
        const std::string cdpath = this->ir_dir + "/cdroot";
        archstream >> my_arch;

        /* REQ: ISO.7 */
        fs::create_directory(target + "/target");
        fs::create_directories(target + "/media/live");

        /* REQ: ISO.9 */
        std::ofstream mtab(target + "/etc/conf.d/mtab");
        if(!mtab) {
            output_error("CD backend", "failed to open mtab configuration");
            return FS_ERROR;
        }
        mtab << "mtab_is_file=no" << std::endl;
        if(mtab.fail() || mtab.bad()) {
            output_error("CD backend", "failed to write mtab configuration");
            return FS_ERROR;
        }
        mtab.close();

        /* REQ: ISO.10 */
        const std::string targetsi = target + "/etc/runlevels/sysinit/";
        fs::create_symlink("/etc/init.d/udev", targetsi + "udev");
        fs::create_symlink("/etc/init.d/udev-trigger",
                           targetsi + "udev-trigger");
        fs::create_symlink("/etc/init.d/lvmetad", targetsi + "lvmetad");

        const std::string squashpath = cdpath + "/" + my_arch + ".squashfs";

        /* REQ: ISO.22 */
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
        if(run_command("mksquashfs", {target, squashpath, "-noappend",
                                      "-wildcards", "-ef",
                                      this->ir_dir + "/exclude.list"}) != 0) {
            output_error("CD backend", "failed to create SquashFS");
            return COMMAND_ERROR;
        }
        return 0;
    }

    int finalise() override {
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
