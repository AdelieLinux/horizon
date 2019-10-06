/*
 * output.hh - Miscellaneous output routines
 * util, the utility library for
 * Project Horizon
 *
 * Copyright (c) 2019 Adélie Linux and contributors.  All rights reserved.
 * This code is licensed under the AGPL 3.0 license, as noted in the
 * LICENSE-code file in the root directory of this repository.
 *
 * SPDX-License-Identifier: AGPL-3.0-only
 */

#ifndef __HORIZON_OUTPUT_HH_
#define __HORIZON_OUTPUT_HH_

#include <string>
#include <iostream>

/*! Prints an error message to the cerr stream.
 * @param where     The location where the error occurred.
 * @param message   The error that occurred.
 * @param detail    Additional detail for the error, if available.
 * @param pretty    Whether or not to colourise (interactive output).
 */
inline void output_error(std::string where, std::string message,
                         std::string detail = "", bool pretty = false) {
    std::cerr << where << ": ";
    if(pretty) std::cerr << "\033[31;1m";
    std::cerr << "error: ";
    if(pretty) std::cerr << "\033[0;1m";
    std::cerr << message;
    if(pretty) std::cerr << "\033[0m";
    if(!detail.empty()) {
        std::cerr << ": " << detail;
    }
    std::cerr << std::endl;
}

#endif /* !__HORIZON_OUTPUT_HH_ */
