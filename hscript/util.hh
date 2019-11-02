/*
 * util.cc - Definition of useful utility routines
 * libhscript, the HorizonScript library for
 * Project Horizon
 *
 * Copyright (c) 2019 Adélie Linux and contributors.  All rights reserved.
 * This code is licensed under the AGPL 3.0 license, as noted in the
 * LICENSE-code file in the root directory of this repository.
 *
 * SPDX-License-Identifier: AGPL-3.0-only
 */

#ifndef HSCRIPT_UTIL_HH
#define HSCRIPT_UTIL_HH

#include <string>

/*! Download the contents of a URL to a path.
 * @param url       The URL to download.
 * @param path      The path in which to save the file.
 * @returns true if successful download, false otherwise.
 */
bool download_file(const std::string &url, const std::string &path);

#endif /* !HSCRIPT_UTIL_HH */
