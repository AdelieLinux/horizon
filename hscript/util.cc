/*
 * util.cc - Implementation of useful utility routines
 * libhscript, the HorizonScript library for
 * Project Horizon
 *
 * Copyright (c) 2019 Adélie Linux and contributors.  All rights reserved.
 * This code is licensed under the AGPL 3.0 license, as noted in the
 * LICENSE-code file in the root directory of this repository.
 *
 * SPDX-License-Identifier: AGPL-3.0-only
 */

#include <string>
#ifdef HAVE_LIBCURL
#    include <cstdio>       /* fopen */
#    include <cstring>      /* strerror */
#    include <curl/curl.h>  /* curl_* */
#    include <errno.h>      /* errno */
#endif /* HAVE_LIBCURL */
#include "util/output.hh"

#ifdef HAVE_LIBCURL
bool download_file(const std::string &url, const std::string &path) {
    CURL *curl = curl_easy_init();
    CURLcode result;
    bool return_code = false;
    char errbuf[CURL_ERROR_SIZE];
    FILE *fp;

    if(curl == nullptr) {
        output_error("internal", "trouble initialising cURL library");
        return false;
    }

    fp = fopen(path.c_str(), "w");
    if(fp == nullptr) {
        output_error("internal", "couldn't open " + path + " for writing",
                     strerror(errno));
        curl_easy_cleanup(curl);
        return false;
    }

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, errbuf);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);

    result = curl_easy_perform(curl);
    if(result == CURLE_OK) {
        return_code = true;
    } else {
        output_error("curl", "couldn't download file", errbuf);
    }
    curl_easy_cleanup(curl);
    return return_code;
}
#else /* !HAVE_LIBCURL */
bool download_file(const std::string &url, const std::string &path) {
    output_error("internal", "can't download without linking to cURL");
    return false;
}
#endif /* HAVE_LIBCURL */
