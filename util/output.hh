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

/*! Bolds output on +stream+ if it's a TTY.
 * @param pretty    Whether to act.
 * @param stream    The stream to turn bold.
 */
inline void bold_if_pretty(bool pretty, std::ostream &stream) {
    if(pretty) stream << "\033[0;1m";
}

/*! ANSI colour code on +stream+ if +pretty+.
 * @param pretty    Whether to act.
 * @param stream    The stream on which to colourise output.
 * @param what      The colour code.
 */
inline void colour_if_pretty(bool pretty, std::ostream &stream,
                             std::string what) {
    if(pretty) stream << "\033[" + what + ";1m";
}

inline void reset_if_pretty(bool pretty, std::ostream &stream) {
    if(pretty) stream << "\033[0m";
}

/*! Outputs a message of the specified +type+ to the log stream.
 * @param type      The type of message to output.
 * @param colour    The colourisation of the message.
 * @param where     The location that triggered the message.
 * @param message   The message.
 * @param detail    Additional detail for the message, if available.
 * @param pretty    Whether or not to colourise (interactive output).
 */
inline void output_message(std::string type, std::string colour,
                           std::string where, std::string message,
                           std::string detail = "", bool pretty = false) {
    std::cerr << where << ": ";
    colour_if_pretty(pretty, std::cerr, colour);
    std::cerr << type << ": ";
    bold_if_pretty(pretty, std::cerr);
    std::cerr << message;
    reset_if_pretty(pretty, std::cerr);
    if(!detail.empty()) {
        std::cerr << ": " << detail;
    }
    std::cerr << std::endl;
}

/*! Outputs an error message.
 * @param where     The location where the error occurred.
 * @param message   The error that occurred.
 * @param detail    Additional detail for the error, if available.
 * @param pretty    Whether or not to colourise (interactive output).
 */
inline void output_error(std::string where, std::string message,
                         std::string detail = "", bool pretty = false) {
    output_message("error", "31", where, message, detail, pretty);
}

/*! Outputs a warning message.
 * @param where     The location where the warning occurred.
 * @param message   The warning to display.
 * @param detail    Additional detail for the warning, if available.
 * @param pretty    Whether or not to colourise (interactive output).
 */
inline void output_warning(std::string where, std::string message,
                           std::string detail = "", bool pretty = false) {
    output_message("warning", "33", where, message, detail, pretty);
}

/*! Outputs an informational message.
 * @param where     The location relevant to the information.
 * @param message   The information to display.
 * @param detail    Additional detail for the information, if available.
 * @param pretty    Whether or not to colourise (interactive output).
 */
inline void output_info(std::string where, std::string message,
                        std::string detail = "", bool pretty = false) {
    output_message("info", "36", where, message, detail, pretty);
}

#endif /* !__HORIZON_OUTPUT_HH_ */
