/*
 * horizonwizard.hh - Definition of the main Wizard class
 * horizon-qt5, the Qt 5 user interface for
 * Project Horizon
 *
 * Copyright (c) 2019 Adélie Linux and contributors.  All rights reserved.
 * This code is licensed under the AGPL 3.0 license, as noted in the
 * LICENSE-code file in the root directory of this repository.
 *
 * SPDX-License-Identifier: AGPL-3.0-only
 */

#ifndef HORIZONWIZARD_HH
#define HORIZONWIZARD_HH

#include <QShortcut>
#include <QWizard>
#include <map>
#include <string>

inline QString fromMacAddress(char address[6]) {
    char buf[18];
    snprintf(buf, 18, "%02X:%02X:%02X:%02X:%02X:%02X",
             address[0] & 0xFF,
             address[1] & 0xFF,
             address[2] & 0xFF,
             address[3] & 0xFF,
             address[4] & 0xFF,
             address[5] & 0xFF);
    QString mac(buf);
    return mac;
}

class HorizonWizard : public QWizard {
public:
    enum {
        Page_Intro,             /* introduction */
        Page_Input,             /* keyboard layout */
        Page_Partition,         /* partitioning */
#ifdef NON_LIBRE_FIRMWARE
        Page_Firmware,          /* firmware */
#endif  /* NON_LIBRE_FIRMWARE */
        Page_Network,           /* network type selection (DHCP/static) */
        Page_Network_Iface,     /* network interface selection */
        Page_Network_Wireless,  /* wireless */
        Page_Network_CustomAP,  /* custom AP */
        Page_Network_DHCP,      /* interstitial for DHCP */
        Page_Network_Portal,    /* shown if captive portal is detected */
        Page_Network_Manual,    /* static addressing */
        Page_DateTime,          /* date and time, TZ, NTP */
        Page_Hostname,          /* hostname */
        Page_PkgSimple,         /* simple package selection */
        Page_PkgCustom,         /* custom package selection */
        Page_PkgCustomDefault,  /* custom package defaults selection */
        Page_Boot,              /* boot loader configuration */
        Page_Root,              /* root passphrase */
        Page_Accounts,          /* user account configuration */
#ifndef HAS_INSTALL_ENV
        Page_Write,             /* write HorizonScript to disk */
#else  /* HAS_INSTALL_ENV */
        Page_Commit             /* begin install */
#endif  /* !HAS_INSTALL_ENV */
    };

    enum NetworkInterfaceType {
        Ethernet,
        Wireless,
        Bonded,
        Unknown
    };

    struct NetworkInterface {
        NetworkInterfaceType type;
        QString mac;
    };

    enum PackageType {
        Standard,
        Mobile,
        Compact,
        TextOnly,
        Custom
    };

    HorizonWizard(QWidget *parent = nullptr);
    void accept();
    /*! Emit a HorizonScript file with the user's choices. */
    QString toHScript();
    QShortcut *esc, *f1, *f3, *f6, *f8;

    /*! The domain to use for downloading packages.
     * @example distfiles.adelielinux.org
     */
    std::string mirror_domain;
    /*! The version of Adélie to install.  Typically "1.0". */
    std::string version;
#ifdef NON_LIBRE_FIRMWARE
    /*! Determines whether firmware will be installed. */
    bool firmware;
#endif  /* NON_LIBRE_FIRMWARE */
    /*! The currently probed network interfaces
     * @note Only available in Installation Environment. */
    std::map<std::string, NetworkInterface> interfaces;
    /*! Determines whether networking will be enabled. */
    bool network;
    /*! Determines whether to use DHCP. */
    bool net_dhcp;
    /*! Determines the network interface to use. */
    std::string chosen_auto_iface;
    /*! Determines the packages to install. */
    PackageType pkgtype;
    /*! If pkgtype is Custom, a list of packages to install. */
    QStringList packages;
    /*! Determines whether to install GRUB. */
    bool grub;
    /*! Determines the kernel to install. */
    std::string kernel;
};

#endif  /* !HORIZONWIZARD_HH */
