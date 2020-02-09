/*
 * net.hh - Miscellaneous networking routines
 * util, the utility library for
 * Project Horizon
 *
 * Copyright (c) 2019 Adélie Linux and contributors.  All rights reserved.
 * This code is licensed under the AGPL 3.0 license, as noted in the
 * LICENSE-code file in the root directory of this repository.
 *
 * SPDX-License-Identifier: AGPL-3.0-only
 */

#ifndef __HORIZON_NET_HH_
#define __HORIZON_NET_HH_

#include <algorithm>
#include <arpa/inet.h>          /* inet_pton */
#include <cstring>              /* memcpy */
#include <string>

/*! Determine if a string starts with a valid, supported protocol
 * @param url       The URL.
 * @returns true if +url+ is a URL for a supported protocol, false otherwise.
 */
static bool is_valid_url(const std::string &url) {
    std::string::size_type colon = url.find("://");
    /* If there's no ://, it's definitely not a URL */
    if(colon == std::string::npos) return false;
    std::string proto = url.substr(0, colon);
    std::transform(proto.cbegin(), proto.cend(), proto.begin(), ::tolower);
    if(proto == "http" || proto == "https" || proto == "tftp" || proto == "smb"
            || proto == "cifs") {
        return true;
    }
    return false;
}

static int subnet_mask_to_cidr(const char *mask) {
    char addr_buf[4];
    uint32_t tmp;
    int real_prefix = ::inet_pton(AF_INET, mask, &addr_buf);

    /* helpfully, we need to init real_prefix to 1 anyway;
     * if inet_pton doesn't return 1, we failed to convert */
    if(real_prefix == 1) {
        memcpy(&tmp, addr_buf, 4);
        tmp = ntohl(tmp);
        while((tmp <<= 1) & 0x80000000) {
            real_prefix++;
        }
    }
    return real_prefix;
}

#endif /* !__HORIZON_NET_HH */
