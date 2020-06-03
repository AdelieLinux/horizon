const char *initrd = "#!/bin/sh -e\n\
        # \n\
        # This file is part of the Horizon image creation system.\n\
        # SPDX-License-Identifier: AGPL-3.0-only\n\
        # \n\
        # Portions of this file are derived from adelie-build-cd.\n\
        # \n\
        \n\
        log() {\n\
            # $1 - Type of log message (info, warning, error)\n\
            # $2 - The message.\n\
            printf \"%s\tlog\tinitrd: %s: %s\n\" `date -u +%Y-%m-%dT%H:%M:%S.000` \"$1\" \"$2\"\n\
        }\n\
        \n\
        log info 'Creating initrd structure...'\n\
        \n\
        ARCH=$1\n\
	LDARCH=$1\n\
	case $ARCH in\n\
	ppc) LDARCH=powerpc;;\n\
	ppc64) LDARCH=powerpc64;;\n\
	pmmx) LDARCH=i386;;\n\
	esac\n\
        TARGET_DIR=$2\n\
        CDROOT_DIR=$3\n\
        CDINIT_DIR=$4\n\
        INITRD_DIR=`mktemp -d -t 'hinitrd-XXXXXX'`\n\
        if [ $? -ne 0 ]; then\n\
            log error 'cannot create temporary directory'\n\
            exit 1\n\
        fi\n\
        \n\
        # mount points\n\
        mkdir ${INITRD_DIR}/dev\n\
        mkdir ${INITRD_DIR}/media\n\
        for _rootdir in newroot lowerroot upperroot; do\n\
            mkdir ${INITRD_DIR}/$_rootdir\n\
            chmod 755 ${INITRD_DIR}/$_rootdir\n\
        done\n\
        mkdir ${INITRD_DIR}/proc\n\
        mkdir ${INITRD_DIR}/sys\n\
        \n\
        # manual /dev nodes for initial udev startup\n\
        mknod -m 600 ${INITRD_DIR}/dev/console c 5 1\n\
        mknod -m 666 ${INITRD_DIR}/dev/null c 1 3\n\
        mknod -m 666 ${INITRD_DIR}/dev/ptmx c 5 2\n\
        mknod -m 666 ${INITRD_DIR}/dev/random c 1 8\n\
        mknod -m 666 ${INITRD_DIR}/dev/tty c 5 0\n\
        mknod -m 620 ${INITRD_DIR}/dev/tty1 c 4 1\n\
        mknod -m 666 ${INITRD_DIR}/dev/urandom c 1 9\n\
        mknod -m 666 ${INITRD_DIR}/dev/zero c 1 5\n\
        \n\
        # base\n\
        mkdir ${INITRD_DIR}/lib\n\
        cp ${TARGET_DIR}/lib/ld-musl-$LDARCH.so.1 ${INITRD_DIR}/lib/\n\
        cp ${TARGET_DIR}/lib/libblkid.so.1 ${INITRD_DIR}/lib/\n\
        cp ${TARGET_DIR}/lib/libuuid.so.1 ${INITRD_DIR}/lib/\n\
        \n\
        # udev\n\
        mkdir -p ${INITRD_DIR}/etc/udev\n\
        mkdir ${INITRD_DIR}/run\n\
        mkdir ${INITRD_DIR}/sbin\n\
        cp ${TARGET_DIR}/bin/udevadm ${INITRD_DIR}/sbin/\n\
        cp ${TARGET_DIR}/sbin/udevd ${INITRD_DIR}/sbin/\n\
        cp ${TARGET_DIR}/lib/libkmod.so.2 ${INITRD_DIR}/lib/\n\
	cp ${TARGET_DIR}/lib/libcrypto.so.1.1 ${INITRD_DIR}/lib/\n\
        cp ${TARGET_DIR}/lib/liblzma.so.5 ${INITRD_DIR}/lib/\n\
        cp ${TARGET_DIR}/lib/libudev.so.1 ${INITRD_DIR}/lib/\n\
        cp ${TARGET_DIR}/lib/libz.so.1 ${INITRD_DIR}/lib/\n\
        \n\
        if [ -n ${CDINIT_DIR} ]; then\n\
                cp ${CDINIT_DIR}/cdinit-$ARCH ${INITRD_DIR}/init\n\
        else\n\
                log error 'No cdinit binary found, and compilation is not yet supported'\n\
                exit 2\n\
        fi\n\
        \n\
        log info 'Compressing initrd...'\n\
        \n\
        (cd ${INITRD_DIR}; find . | cpio -H newc -o) > ${CDROOT_DIR}/initrd-$ARCH\n\
        gzip -9 ${CDROOT_DIR}/initrd-$ARCH\n\
        mv ${CDROOT_DIR}/initrd-$ARCH.gz ${CDROOT_DIR}/initrd-$ARCH";
