/*
 * output.hh - Miscellaneous output routines
 * util, the utility library for
 * Project Horizon
 *
 * Copyright (c) 2019-2020 Adélie Linux and contributors.  All rights reserved.
 * This code is licensed under the AGPL 3.0 license, as noted in the
 * LICENSE-code file in the root directory of this repository.
 *
 * SPDX-License-Identifier: AGPL-3.0-only
 */

#ifndef __HORIZON_OUTPUT_HH_
#define __HORIZON_OUTPUT_HH_

extern bool pretty;

#include <string>
#include <iostream>
#include <chrono>
#include <iomanip>

#include <hscript/script_l.hh>

/*! Bolds output on +stream+ if pretty output is desired.
 * @param stream    The stream to turn bold.
 */
inline void bold_if_pretty(std::ostream &stream) {
    if(pretty) stream << "\033[0;1m";
}

/*! ANSI colour code on +stream+ if pretty output is desired.
 * @param stream    The stream on which to colourise output.
 * @param what      The colour code.
 */
inline void colour_if_pretty(std::ostream &stream, const std::string &what) {
    if(pretty) stream << "\033[" + what + ";1m";
}

/*! Reset all formatting on +stream+ if pretty output is desired.
 * @param stream    The stream on which to reset formatting.
 */
inline void reset_if_pretty(std::ostream &stream) {
    if(pretty) stream << "\033[0m";
}

/*! Print the ISO 8601 timestamp with millisecond resolution.
 * @note See Agent.MessageFormat.
 */
inline void output_time() {
    using namespace std::chrono;
    auto now = system_clock::now();
    auto millis = duration_cast<milliseconds>(now.time_since_epoch()) % 1000;
    auto time = system_clock::to_time_t(now);
    std::tm utc = *std::gmtime(&time);
    std::cerr << std::put_time(&utc, "%FT%T") << ".";
    std::cerr << std::setfill('0') << std::setw(3) << millis.count();
}

/*! Outputs that +step+ is beginning execution. */
inline void output_step_start(const std::string &step) {
    output_time();
    std::cerr << "\tstep-start\t";
    std::cerr << step << std::endl;
}

/*! Outputs that +step+ is finishing execution. */
inline void output_step_end(const std::string &step) {
    output_time();
    std::cerr << "\tstep-end\t";
    std::cerr << step << std::endl;
}

/*! Outputs a message of the specified +type+ to the log stream.
 * @param type      The type of message to output.
 * @param colour    The colourisation of the message.
 * @param where     The location that triggered the message.
 * @param message   The message.
 * @param detail    Additional detail for the message, if available.
 */
inline void output_log(const std::string &type, const std::string &colour,
                       const std::string &where, const std::string &message,
                       const std::string &detail = "") {
    output_time();
    std::cerr << "\tlog\t";
    std::cerr << where << ": ";
    colour_if_pretty(std::cerr, colour);
    std::cerr << type << ": ";
    bold_if_pretty(std::cerr);
    std::cerr << message;
    reset_if_pretty(std::cerr);
    if(!detail.empty()) {
        std::cerr << ": " << detail;
    }
    std::cerr << std::endl;
}

/*! Outputs an error message.
 * @param where     The location where the error occurred.
 * @param message   The error that occurred.
 * @param detail    Additional detail for the error, if available.
 */
inline void output_error(const std::string &where, const std::string &message,
                         const std::string &detail = "") {
    output_log("error", "31", where, message, detail);
}

/*! Outputs an error message.
 * @param where     The location where the error occurred.
 * @param message   The error that occurred.
 * @param detail    Additional detail for the error, if available.
 */
inline void output_error(const Horizon::ScriptLocation &where,
                         const std::string &message,
                         const std::string &detail = "") {
    output_error(where.name + ":" + std::to_string(where.line), message,
                 detail);
}

/*! Outputs a warning message.
 * @param where     The location where the warning occurred.
 * @param message   The warning to display.
 * @param detail    Additional detail for the warning, if available.
 */
inline void output_warning(const std::string &where, const std::string &message,
                           const std::string &detail = "") {
    output_log("warning", "33", where, message, detail);
}

/*! Outputs a warning message.
 * @param where     The location where the warning occurred.
 * @param message   The warning to display.
 * @param detail    Additional detail for the warning, if available.
 */
inline void output_warning(const Horizon::ScriptLocation &where,
                           const std::string &message,
                           const std::string &detail = "") {
    output_warning(where.name + ":" + std::to_string(where.line), message,
                   detail);
}

/*! Outputs an informational message.
 * @param where     The location relevant to the information.
 * @param message   The information to display.
 * @param detail    Additional detail for the information, if available.
 */
inline void output_info(const std::string &where, const std::string &message,
                        const std::string &detail = "") {
    output_log("info", "36", where, message, detail);
}

/*! Outputs an informational message.
 * @param where     The location relevant to the information.
 * @param message   The information to display.
 * @param detail    Additional detail for the information, if available.
 */
inline void output_info(const Horizon::ScriptLocation &where,
                        const std::string &message,
                        const std::string &detail = "") {
    output_info(where.name + ":" + std::to_string(where.line), message, detail);
}

#endif /* !__HORIZON_OUTPUT_HH_ */
