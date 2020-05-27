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
#include <vector>
#ifdef HAVE_LIBCURL
#    include <cstdio>           /* fopen */
#    include <cstring>          /* strerror */
#    include <curl/curl.h>      /* curl_* */
#    include <errno.h>          /* errno */
#endif /* HAVE_LIBCURL */
#ifdef HAS_INSTALL_ENV
#   include <spawn.h>           /* posix_spawnp */
#   include <sys/wait.h>        /* waitpid, W* */
#   include <unistd.h>          /* environ */
#endif
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
        fflush(fp);
        return_code = true;
    } else {
        output_error("curl", "couldn't download file", errbuf);
    }

    fclose(fp);
    curl_easy_cleanup(curl);
    return return_code;
}
#else /* !HAVE_LIBCURL */
bool download_file(const std::string &url, const std::string &path) {
    output_error("internal", "can't download without linking to cURL");
    return false;
}
#endif /* HAVE_LIBCURL */

int run_command(const std::string &cmd, const std::vector<std::string> &args) {
#ifdef HAS_INSTALL_ENV
    const char **argv = new const char*[args.size() + 2];
    pid_t child;
    int status;

    argv[0] = cmd.c_str();
    for(unsigned long index = 0; index < args.size(); index++) {
        argv[index + 1] = args.at(index).c_str();
    }
    argv[args.size() + 1] = nullptr;

    status = posix_spawnp(&child, cmd.c_str(), nullptr, nullptr,
                          const_cast<char * const *>(argv), environ);
    if(status != 0) {
        /* extremely unlikely failure case */
        output_error(cmd, "cannot fork", strerror(status));
        delete[] argv;
        return -1;
    }

    delete[] argv;

    if(waitpid(child, &status, 0) == -1) {
        /* unlikely failure case */
        output_error(cmd, "waitpid", strerror(errno));
        return -1;
    }

    if(!WIFEXITED(status)) {
        output_error(cmd, "received fatal signal " +
                     std::to_string(WTERMSIG(status)));
        return -1;
    }

    if(WEXITSTATUS(status) != 0) {
        output_error(cmd, "exited abnormally with status " +
                     std::to_string(WEXITSTATUS(status)));
    }

    return WEXITSTATUS(status);
#else /* !HAS_INSTALL_ENV */
    output_error(cmd, "can't spawn processes in runtine environment");
    return -1;
#endif /* HAS_INSTALL_ENV */
}
