/*
 * fetch.cc - Implementation of the HorizonScript Locator
 * Project Horizon
 *
 * Copyright (c) 2019 Adélie Linux and contributors.  All rights reserved.
 * This code is licensed under the AGPL 3.0 license, as noted in the
 * LICENSE-code file in the root directory of this repository.
 *
 * SPDX-License-Identifier: AGPL-3.0-only
 */

#include <algorithm>        /* transform */
#include <cstdlib>          /* EXIT_* */
#ifdef HAVE_LIBCURL
#    include <cstring>      /* strerror */
#    include <curl/curl.h>  /* curl_* */
#    include <errno.h>      /* errno */
#endif /* HAVE_LIBCURL */
#include <fstream>
#include <map>
#include <string>
#include <unistd.h>         /* access */
#include "util/output.hh"

static const char *IFILE_PATH = "/etc/horizon/installfile";
static const int SCRIPT_LINE_MAX = 512;

bool pretty = true;

/*! Process a local path and copy it to the proper location.
 * @param path      The local path.
 * @returns An exit code.
 */
int process_local(const std::string &path) {
    /* if we can't read it, don't even bother */
    if(access(path.c_str(), R_OK) != 0) {
        output_error("process_local",
                     std::string(path) + " does not exist or is not readable");
        return EXIT_FAILURE;
    }

    std::ofstream output(IFILE_PATH, std::ios_base::trunc);
    if(!output) {
        output_error("process_local",
                     std::string(IFILE_PATH) +
                     " could not be opened for writing");
        return EXIT_FAILURE;
    }
    std::ifstream input(path, std::ios_base::in);
    if(!input) {
        output_error("process_local",
                     path + " could not be opened for reading");
        return EXIT_FAILURE;
    }
    char buffer[SCRIPT_LINE_MAX];

    while(input.getline(buffer, SCRIPT_LINE_MAX)) {
        output << buffer << std::endl;
    }

    if(input.fail() && !input.eof()) {
        output_error("process_local", "line too long while reading " + path);
        return EXIT_FAILURE;
    }
    if(input.bad() && !input.eof()) {
        output_error("process_local", "I/O error reading " + path);
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}


#ifdef HAVE_LIBCURL
/*! Download an installfile using cURL.
 * @param path      The remote path to download.
 */
int process_curl(const std::string &path) {
    CURL *curl = curl_easy_init();
    CURLcode result;
    int return_code = EXIT_FAILURE;
    char errbuf[CURL_ERROR_SIZE];
    FILE *fp;

    if(curl == nullptr) {
        output_error("internal", "trouble initialising cURL library");
        return EXIT_FAILURE;
    }

    fp = fopen(IFILE_PATH, "w");
    if(fp == nullptr) {
        output_error("internal", "couldn't open " + std::string(IFILE_PATH) +
                     " for writing", strerror(errno));
        curl_easy_cleanup(curl);
        return EXIT_FAILURE;
    }

    curl_easy_setopt(curl, CURLOPT_URL, path.c_str());
    curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, errbuf);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);

    result = curl_easy_perform(curl);
    if(result == CURLE_OK) {
        return_code = EXIT_SUCCESS;
    } else {
        output_error("curl", "couldn't download installfile", errbuf);
    }
    curl_easy_cleanup(curl);
    return return_code;
}
#endif /* HAVE_LIBCURL */


/*! The signature of a protocol dispatch function */
typedef int (*proto_dispatch_fn)(const std::string &);


/*! Protocol handlers */
static const std::map<std::string, proto_dispatch_fn> protos = {
#ifdef HAVE_LIBCURL
    {"http", process_curl},
    {"https", process_curl},
    {"tftp", process_curl},
#endif /* HAVE_LIBCURL */
};


/*! Dispatch handling of +path+ to a protocol handler, or return failure.
 * @param path      The remote path to download to IFILE_PATH.
 * @returns EXIT_SUCCESS if the file was downloaded; EXIT_FAILURE otherwise.
 * @note Currently, it is ambiguous whether EXIT_FAILURE means invalid proto
 * or error downloading from the URL.
 */
int process_maybe_remote(const std::string &path) {
    std::string proto = path.substr(0, path.find_first_of(":"));
    std::transform(proto.begin(), proto.end(), proto.begin(), ::tolower);

    if(protos.find(proto) != protos.end()) {
        return (*protos.at(proto))(path);
    }

    std::string support = "This build of Horizon supports: ";
    for(auto &proto_pair : protos) {
        support += proto_pair.first + " ";
    }

    output_error("argv", proto + " is not a valid protocol", support);
    return EXIT_FAILURE;
}


/*! Figure out the TFTP path(s) to try, and use them */
int full_auto_tftp() {
    output_error("tftp", "stub!");
    return EXIT_FAILURE;
}


/*
 * The goal of the Locator is to find the HorizonScript for this computer,
 * which is the target, and copy it to the well-known path /etc/horizon
 * as 'installfile'.
 */
int main(int argc, char *argv[]) {
    std::string installfile;

    if(argc < 1 || argc > 2) {
        std::cerr << "usage: " << std::endl;
        std::cerr << "\thscript-fetch [path|url]" << std::endl;
        return EXIT_FAILURE;
    }

    bold_if_pretty(std::cout);
    std::cout << "HorizonScript Locator version " << VERSTR;
    reset_if_pretty(std::cout);
    std::cout << std::endl
              << "Copyright (c) 2019-2020 Adélie Linux and contributors."
              << std::endl
              << "This software is licensed to you under the terms of the "
              << std::endl << "AGPL 3.0 license, unless otherwise noted."
              << std::endl << std::endl;

    if(argc == 1) {
        if(access(IFILE_PATH, F_OK) == 0) {
            /* That was easy™ */
            std::cout << "HorizonScript already present at " << IFILE_PATH
                      << ".  Quitting." << std::endl;
            return EXIT_SUCCESS;
        }

        if(protos.find("tftp") == protos.end()) {
            output_error("internal", "A Fully Automatic installation requires "
                         "Horizon to be built with TFTP support");
            return EXIT_FAILURE;
        } else {
            return full_auto_tftp();
        }
    }

    std::string path(argv[1]);
    if(path == "--version") {
        /* Banner printed above is our version statement. */
        return EXIT_SUCCESS;
    }

    if(path[0] == '/') {
        return process_local(path);
    }
    if(path.find("://") != std::string::npos) {
        return process_maybe_remote(path);
    }

    output_error("argv", "Unrecognised protocol or invalid URL",
                 "An absolute path or URL is required");
    return EXIT_FAILURE;
}
