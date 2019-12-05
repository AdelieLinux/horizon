/*
 * keymaps.hh - Keymap definitions
 * util, the utility library for
 * Project Horizon
 *
 * Copyright (c) 2019 Adélie Linux and contributors.  All rights reserved.
 * This code is licensed under the AGPL 3.0 license, as noted in the
 * LICENSE-code file in the root directory of this repository.
 *
 * SPDX-License-Identifier: AGPL-3.0-only
 */

#ifndef HORIZON_KEYMAPS_HH_
#define HORIZON_KEYMAPS_HH_

#include <set>
#include <string>

const std::set<std::string> valid_keymaps = {
    "us", "ad", "af", "ara", "al", "am", "at", "az", "by", "be", "bd", "in",
    "ba", "br", "bg", "ma", "mm", "ca", "cd", "cn", "hr", "cz", "dk", "nl",
    "bt", "ee", "ir", "iq", "fo", "fi", "fr", "gh", "gn", "ge", "de", "gr",
    "hu", "is", "il", "it", "jp", "kg", "kh", "kz", "la", "latam", "lt", "lv",
    "mao", "me", "mk", "mt", "mn", "no", "pl", "pt", "ro", "ru", "rs", "si",
    "sk", "es", "se", "ch", "sy", "tj", "lk", "th", "tr", "ua", "gb", "uz",
    "vn", "kr", "ie", "pk", "mv", "za", "epo", "np", "ng", "et", "sn", "brai",
    "tm", "ml", "tz", "ke", "bw", "ph"
};

#endif  /* !HORIZON_KEYMAPS_HH_ */
