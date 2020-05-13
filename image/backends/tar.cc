/*
 * tar.cc - Implementation of the tarball Horizon Image Creation backend
 * image, the image processing utilities for
 * Project Horizon
 *
 * Copyright (c) 2020 Adélie Linux and contributors.  All rights reserved.
 * This code is licensed under the AGPL 3.0 license, as noted in the
 * LICENSE-code file in the root directory of this repository.
 *
 * SPDX-License-Identifier: AGPL-3.0-only
 */

#include <archive.h>
#include <archive_entry.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include "basic.hh"
#include "util/filesystem.hh"
#include "util/output.hh"

namespace Horizon {
namespace Image {

class TarBackend : public BasicBackend {
public:
    enum CompressionType {
        None,
        GZip,
        BZip2,
        XZ
    };

private:
    CompressionType comp;
    struct archive *a;

public:
    TarBackend(std::string ir, std::string out, CompressionType _c = None)
        : BasicBackend(ir, out), comp(_c) {};

    int prepare() override {
        int res;

        a = archive_write_new();
        archive_write_set_format_pax_restricted(a);

        switch(comp) {
        case None:
            break;
        case GZip:
            archive_write_add_filter_gzip(a);
            break;
        case BZip2:
            archive_write_add_filter_bzip2(a);
            break;
        case XZ:
            archive_write_add_filter_xz(a);
            break;
        }

        res = archive_write_open_filename(a, this->out_path.c_str());
        if(res < ARCHIVE_OK) {
            if(res < ARCHIVE_WARN) {
                output_error("tar backend", archive_error_string(a));
                return res;
            } else {
                output_warning("tar backend", archive_error_string(a));
            }
        }

        return 0;
    }

    int create() override {
        struct archive_entry *entry = archive_entry_new();
        error_code ec;
        int fd, r, code = 0;
        struct stat s;
        void *buff;

        for(const auto& dent : fs::recursive_directory_iterator(this->ir_dir,
                                                                ec)) {
            fs::path relpath = dent.path().lexically_relative(this->ir_dir);
#define OUTPUT_FAILURE(x) \
    output_error("tar backend", "failed to " x " '" + std::string(dent.path()) + "'",\
                 strerror(errno));
            r = lstat(dent.path().c_str(), &s);
            if(r == -1) {
                OUTPUT_FAILURE("stat")
                code = -1;
                goto ret;
            }
            archive_entry_copy_stat(entry, &s);
            if(dent.is_symlink()) {
                archive_entry_set_filetype(entry, AE_IFLNK);
                fs::path resolved = fs::read_symlink(dent.path(), ec);
                const fs::path::value_type *c_rpath = resolved.u8string().c_str();
                archive_entry_update_symlink_utf8(entry, c_rpath);
            }
            archive_entry_update_pathname_utf8(entry, relpath.u8string().c_str());
            if(archive_write_header(this->a, entry) != ARCHIVE_OK) {
                output_error("tar backend", archive_error_string(a));
                code = -1;
                goto ret;
            }
            if(dent.is_regular_file() && dent.file_size() > 0) {
                fd = open(dent.path().c_str(), O_RDONLY);
                if(fd == -1) {
                    OUTPUT_FAILURE("open")
                    code = -1;
                    goto ret;
                }
                buff = mmap(NULL, dent.file_size(), PROT_READ, MAP_SHARED,
                            fd, 0);
                if(buff == MAP_FAILED) {
                    OUTPUT_FAILURE("map")
                    close(fd);
                    code = -1;
                    goto ret;
                }
                archive_write_data(this->a, buff, dent.file_size());
                close(fd);
            }
            archive_write_finish_entry(this->a);
            archive_entry_clear(entry);
        }

ret:
        archive_entry_free(entry);
        return code;
    }

    int finalise() override {
        archive_write_close(a);
        archive_write_free(a);

        return 0;
    }
};

__attribute__((constructor(400)))
void register_tar_backend() {
    BackendManager::register_backend(
    {"tar", "Create a tarball (.tar)",
        [](std::string ir_dir, std::string out_path) {
            return new TarBackend(ir_dir, out_path);
        }
    });

    BackendManager::register_backend(
    {"tgz", "Create a tarball with GZ compression (.tar.gz)",
        [](std::string ir_dir, std::string out_path) {
            return new TarBackend(ir_dir, out_path, TarBackend::GZip);
        }
    });

    BackendManager::register_backend(
    {"tbz", "Create a tarball with BZip2 compression (.tar.bz2)",
        [](std::string ir_dir, std::string out_path) {
            return new TarBackend(ir_dir, out_path, TarBackend::BZip2);
        }
    });

    BackendManager::register_backend(
    {"txz", "Create a tarball with XZ compression (.tar.xz)",
        [](std::string ir_dir, std::string out_path) {
            return new TarBackend(ir_dir, out_path, TarBackend::XZ);
        }
    });
}

}
}
