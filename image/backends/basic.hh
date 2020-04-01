/*
 * basic.hh - Definition of the abstract Horizon Image Creation backend
 * image, the image processing utilities for
 * Project Horizon
 *
 * Copyright (c) 2020 Adélie Linux and contributors.  All rights reserved.
 * This code is licensed under the AGPL 3.0 license, as noted in the
 * LICENSE-code file in the root directory of this repository.
 *
 * SPDX-License-Identifier: AGPL-3.0-only
 */

#pragma once

#include <string>

namespace Horizon {
namespace Image {

class BasicBackend {
public:
    /*! Create the backend object.
     * @param _ir_dir   The intermediate directory the image should contain.
     * @param _out_path The path to write the image.
     */
    BasicBackend(const std::string &_ir_dir, const std::string &_out_path) :
        ir_dir(_ir_dir), out_path(_out_path) {};
    virtual ~BasicBackend() {};

    /*! Prepare for creating the image.
     * @returns 0 if preparation succeeded; otherwise, an error code.
     */
    virtual int prepare();

    /*! Create the image.
     * @returns 0 if the image is created successfully; otherwise, an error
     *          code which explains the failure.
     */
    virtual int create() = 0;

    /*! Finalise the image.
     * @returns 0 if the image is finalised; otherwise, an error code.
     */
    virtual int finalise();

    /*! The intermediate directory which contains the sysroot the image
     *  should contain. */
    const std::string &ir_dir;
    /*! The path at which to write the image. */
    const std::string &out_path;
};

}
}
