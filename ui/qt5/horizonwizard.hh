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

class HorizonWizard : public QWizard {
public:
    enum {
        Page_Intro,             /* introduction */
        Page_Input,             /* keyboard layout */
        Page_Partition,         /* partitioning */
#ifdef NON_LIBRE_FIRMWARE
        Page_Firmware,          /* firmware */
#endif  /* NON_LIBRE_FIRMWARE */
#ifndef HAS_INSTALL_ENV
        Page_Network_Define,    /* define network interfaces for target */
#endif  /* !HAS_INSTALL_ENV */
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

    HorizonWizard(QWidget *parent = nullptr);
    QShortcut *f1, *f3, *f5, *f8;

    std::map<std::string, NetworkInterfaceType> interfaces;
    std::string chosen_auto_iface;
};

#endif  /* !HORIZONWIZARD_HH */
