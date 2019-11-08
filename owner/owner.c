/*
 * owner.c - Implementation of the "print file owner UID" utility
 * Project Horizon
 *
 * Copyright (c) 2019 Adélie Linux and contributors.  All rights reserved.
 * This code is licensed under the NCSA license.
 *
 * SPDX-License-Identifier: NCSA
 */

#include <stdio.h>      /* fprintf, perror */
#include <stdlib.h>     /* EXIT_* */
#include <sys/stat.h>   /* stat(3) and friends */

int main(int argc, char *argv[]) {
    if(argc != 2) {
        fprintf(stderr, "hscript-printowner: requires a path or file\n");
        return EXIT_FAILURE;
    }

    struct stat buf;
    if(stat(argv[1], &buf) != 0) {
        perror("hscript-printowner: stat");
        return EXIT_FAILURE;
    }

    fprintf(stdout, "%d\n", buf.st_uid);

    return EXIT_SUCCESS;
}
