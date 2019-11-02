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
#include <vector>

/*! Download the contents of a URL to a path.
 * @param url       The URL to download.
 * @param path      The path in which to save the file.
 * @returns true if successful download, false otherwise.
 */
bool download_file(const std::string &url, const std::string &path);

/*! Run a command.
 * @param cmd       The command to run.
 * @param args      Arguments to pass to the command.
 * @returns 0 if the command exited normally with status 0,
 * the exit code if the command exited abnormally,
 * -1 if the command signalled.
 * @note Status of the command is output using +output_error+.
 */
int run_command(const std::string &cmd, const std::vector<std::string> &args);

#endif /* !HSCRIPT_UTIL_HH */
