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
#include "basic.hh"
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

        res = archive_write_open_filename(a, this->out_path.c_str()) < ARCHIVE_OK;
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
        return 0;
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
