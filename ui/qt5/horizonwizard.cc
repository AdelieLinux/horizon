/*
 * horizonwizard.cc - Implementation of the main Wizard class
 * horizon-qt5, the Qt 5 user interface for
 * Project Horizon
 *
 * Copyright (c) 2019 Adélie Linux and contributors.  All rights reserved.
 * This code is licensed under the AGPL 3.0 license, as noted in the
 * LICENSE-code file in the root directory of this repository.
 *
 * SPDX-License-Identifier: AGPL-3.0-only
 */

#include "horizonwizard.hh"
#include "horizonhelpwindow.hh"

#include <QDebug>
#include <QFile>
#include <QMessageBox>
#include <openssl/rand.h>

#include <algorithm>
#include <map>
#include <string>

#ifdef HAS_INSTALL_ENV
#   include <libudev.h>
#   include <net/if.h>      /* ifreq */
extern "C" {
#   include <skalibs/tai.h> /* STAMP */
}
#   include <sys/ioctl.h>   /* ioctl */
#   include <unistd.h>      /* close */
#endif  /* HAS_INSTALL_ENV */

#include "intropage.hh"
#include "inputpage.hh"
#ifdef NON_LIBRE_FIRMWARE
#include "firmwarepage.hh"
#endif  /* NON_LIBRE_FIRMWARE */
#include "networkingpage.hh"
#include "networkifacepage.hh"
#include "netsimplewifipage.hh"
#include "netdhcppage.hh"
#include "datetimepage.hh"
#include "hostnamepage.hh"
#include "pkgsimple.hh"
#include "bootpage.hh"
#include "rootpwpage.hh"
#include "accountpage.hh"

#include "util/keymaps.hh"

static std::map<int, std::string> help_id_map = {
    {HorizonWizard::Page_Intro, "intro"},
    {HorizonWizard::Page_Input, "input"},
    {HorizonWizard::Page_Partition, "partition"},
#ifdef NON_LIBRE_FIRMWARE
    {HorizonWizard::Page_Firmware, "firmware"},
#endif  /* NON_LIBRE_FIRMWARE */
    {HorizonWizard::Page_Network, "network-start"},
    {HorizonWizard::Page_Network_Iface, "network-iface"},
    {HorizonWizard::Page_Network_Wireless, "network-wifi"},
    {HorizonWizard::Page_Network_CustomAP, "network-wifi-ap"},
    {HorizonWizard::Page_Network_DHCP, "network-dhcp"},
    {HorizonWizard::Page_Network_Portal, "network-portal"},
    {HorizonWizard::Page_Network_Manual, "network-manual"},
    {HorizonWizard::Page_DateTime, "datetime"},
    {HorizonWizard::Page_Hostname, "hostname"},
    {HorizonWizard::Page_PkgSimple, "packages-simple"},
    {HorizonWizard::Page_PkgCustom, "packages-custom"},
    {HorizonWizard::Page_PkgCustomDefault, "packages-custom-default"},
    {HorizonWizard::Page_Boot, "startup"},
    {HorizonWizard::Page_Root, "rootpw"},
    {HorizonWizard::Page_Accounts, "accounts"},
#ifndef HAS_INSTALL_ENV
    {HorizonWizard::Page_Write, "writeout"},
#else  /* HAS_INSTALL_ENV */
    {HorizonWizard::Page_Commit, "commit"}
#endif  /* !HAS_INSTALL_ENV */
};


#ifdef HAS_INSTALL_ENV
std::map<std::string, HorizonWizard::NetworkInterface> probe_ifaces(void) {
    struct udev *udev;
    struct udev_enumerate *if_list;
    struct udev_list_entry *first, *candidate;
    struct udev_device *device = nullptr;

    std::map<std::string, HorizonWizard::NetworkInterface> ifaces;

    udev = udev_new();
    if(udev == nullptr) {
        qDebug() << "Can't connect to UDev.";
        return ifaces;
    }

    if_list = udev_enumerate_new(udev);
    if(if_list == nullptr) {
        qDebug() << "Uh oh.  UDev is unhappy.";
        udev_unref(udev);
        return ifaces;
    }

    udev_enumerate_add_match_subsystem(if_list, "net");
    udev_enumerate_scan_devices(if_list);
    first = udev_enumerate_get_list_entry(if_list);
    udev_list_entry_foreach(candidate, first) {
        const char *syspath = udev_list_entry_get_name(candidate);
        const char *devtype, *cifname;
        QString mac;

        if(device != nullptr) udev_device_unref(device);
        device = udev_device_new_from_syspath(udev, syspath);
        if(device == nullptr) continue;
        devtype = udev_device_get_devtype(device);
        if(devtype == nullptr) {
            devtype = udev_device_get_sysattr_value(device, "type");
            if(devtype == nullptr) {
                qDebug() << syspath << " skipped; no device type.";
                continue;
            }
        }

        cifname = udev_device_get_property_value(device, "INTERFACE");
        if(cifname == nullptr) {
            qDebug() << syspath << " has no interface name.";
            continue;
        }

        /* Retrieving the index is always valid, and is not even privileged. */
        struct ifreq request;
        int my_sock = ::socket(AF_INET, SOCK_STREAM, 0);
        if(my_sock != -1) {
            memset(&request, 0, sizeof(request));
            memcpy(&request.ifr_name, cifname, strnlen(cifname, IFNAMSIZ));
            errno = 0;
            if(ioctl(my_sock, SIOCGIFHWADDR, &request) != -1) {
                mac = fromMacAddress(request.ifr_ifru.ifru_hwaddr.sa_data);
            }
            ::close(my_sock);
        }

        std::string ifname(cifname);
        if(strstr(devtype, "wlan")) {
            ifaces.insert({ifname, {HorizonWizard::Wireless, mac}});
        } else if(strstr(devtype, "bond")) {
            ifaces.insert({ifname, {HorizonWizard::Bonded, mac}});
        } else if(strstr(syspath, "/virtual/")) {
            /* Skip lo, tuntap, etc */
            continue;
        } else if(strstr(devtype, "1")) {
            ifaces.insert({ifname, {HorizonWizard::Ethernet, mac}});
        } else {
            ifaces.insert({ifname, {HorizonWizard::Unknown, mac}});
        }
    }

    if(device != nullptr) udev_device_unref(device);

    udev_enumerate_unref(if_list);
    udev_unref(udev);
    return ifaces;
}
#endif  /* HAS_INSTALL_ENV */


HorizonWizard::HorizonWizard(QWidget *parent) : QWizard(parent) {
    setWindowTitle(tr("Adélie Linux System Installation"));

    setFixedSize(QSize(650, 450));

    mirror_domain = "distfiles.adelielinux.org";
    version = "stable";
    /* TODO XXX:
     * Determine which platform kernel is being used, if any (-power8 etc)
     * Determine hardware requirements (easy or mainline)
     */
    kernel = "easy-kernel";

    /* REQ: UI.Global.Back.Save */
    setOption(IndependentPages);
    /* REQ: UI.Language.Buttons, Iface.UI.StandardButtons */
    setOption(HaveHelpButton);
    setOption(NoBackButtonOnStartPage);

    setSizeGripEnabled(false);

    setPage(Page_Intro, new IntroPage);
    setPage(Page_Input, new InputPage);
#ifdef NON_LIBRE_FIRMWARE
    setPage(Page_Firmware, new FirmwarePage);
#endif  /* NON_LIBRE_FIRMWARE */
    setPage(Page_Network, new NetworkingPage);
    setPage(Page_Network_Iface, new NetworkIfacePage);
    setPage(Page_Network_Wireless, new NetworkSimpleWirelessPage);
    setPage(Page_Network_DHCP, new NetDHCPPage);
    setPage(Page_DateTime, new DateTimePage);
    setPage(Page_Hostname, new HostnamePage);
    setPage(Page_PkgSimple, new PkgSimplePage);
    setPage(Page_Boot, new BootPage);
    setPage(Page_Root, new RootPassphrasePage);
    setPage(Page_Accounts, new AccountPage);

    QObject::connect(this, &QWizard::helpRequested, [=](void) {
        if(help_id_map.find(currentId()) == help_id_map.end()) {
            qDebug() << "no help available for " << currentId();
            QMessageBox nohelp(QMessageBox::Warning,
                               tr("No Help Available"),
                               tr("Help is not available for the current page."
                                  "  Consult the Installation Guide for more "
                                  "information."),
                               QMessageBox::Ok,
                               this);
            nohelp.exec();
            return;
        }
        std::string helppath = ":/wizard_help/resources/" +
                               help_id_map.at(currentId()) + "-help.txt";
        QFile helpfile(helppath.c_str());
        helpfile.open(QFile::ReadOnly);
        HorizonHelpWindow help(&helpfile, this);
        help.exec();
    });

    /* REQ: UI.Global.Cancel.Confirm */
    button(CancelButton)->disconnect(this);
    QObject::connect(button(CancelButton), &QAbstractButton::clicked,
                     [=](bool) {
        QMessageBox cancel(QMessageBox::Question,
                           tr("Exit Adélie Linux System Installation"),
                   #ifdef HAS_INSTALL_ENV
                           tr("Adélie Linux has not yet been set up on your computer.  Exiting will reboot your computer.  Do you wish to exit?"),
                   #else
                           tr("You have not yet completed the System Installation wizard.  No installfile will be generated.  Do you wish to exit?"),
                   #endif
                           QMessageBox::Yes | QMessageBox::No,
                           this);
        cancel.setDefaultButton(QMessageBox::No);
        if(cancel.exec() == QMessageBox::Yes) {
            reject();
        }
    });

    /* REQ: Iface.UI.ButtonAccel */
    setButtonText(HelpButton, tr("&Help (F1)"));
    setButtonText(CancelButton, tr("E&xit (F3)"));
    setButtonText(BackButton, tr("< &Back (F6)"));
    setButtonText(NextButton, tr("Co&ntinue > (F8)"));
#ifdef HAS_INSTALL_ENV
    setButtonText(FinishButton, tr("&Install (F8)"));
#else
    setButtonText(FinishButton, tr("&Save (F8)"));
#endif  /* HAS_INSTALL_ENV */

    f1 = new QShortcut(Qt::Key_F1, this);
    connect(f1, &QShortcut::activated,
            button(HelpButton), &QAbstractButton::click);
    f1->setWhatsThis(tr("Activates the Help screen."));

    esc = new QShortcut(Qt::Key_Escape, this);
    connect(esc, &QShortcut::activated,
            button(CancelButton), &QAbstractButton::click);
    f3 = new QShortcut(Qt::Key_F3, this);
    connect(f3, &QShortcut::activated,
            button(CancelButton), &QAbstractButton::click);
    f3->setWhatsThis(tr("Prompts to exit the installation."));

    f6 = new QShortcut(Qt::Key_F6, this);
    connect(f6, &QShortcut::activated,
            button(BackButton), &QAbstractButton::click);
    f6->setWhatsThis(tr("Goes back to the previous page."));

    f8 = new QShortcut(Qt::Key_F8, this);
    connect(f8, &QShortcut::activated,
            button(NextButton), &QAbstractButton::click);
    connect(f8, &QShortcut::activated,
            button(FinishButton), &QAbstractButton::click);
    f8->setWhatsThis(tr("Goes forward to the next page."));

#ifdef HAS_INSTALL_ENV
    interfaces = probe_ifaces();
    tain_now_set_stopwatch_g();
#endif  /* HAS_INSTALL_ENV */
}


extern "C" char *do_a_crypt(const char *key, const char *setting, char *output);

/*! Make a shadow-compliant crypt string out of +the_pw+. */
char *encrypt_pw(const char *the_pw) {
    unsigned char rawsalt[8];
    RAND_bytes(rawsalt, 8);
    QByteArray salt_ba(reinterpret_cast<char *>(rawsalt), 8);

    const char *salt = ("$6$" + salt_ba.toBase64(QByteArray::OmitTrailingEquals)
                       .toStdString() + "$").c_str();
    char *result = new char[128];
    if(do_a_crypt(the_pw, salt, result) == nullptr) {
        delete[] result;
        return nullptr;
    }
    return result;
}


QString HorizonWizard::toHScript() {
    QStringList lines;

    if(this->network) {
        lines << "network true";

        if(this->net_dhcp) {
            lines << QString::fromStdString("netaddress " +
                                            this->chosen_auto_iface + " dhcp");
        } else {
            /* XXX TODO no manual page implemented yet */
            Q_ASSERT(false);
        }
    } else {
        lines << "network false";
    }

    lines << ("hostname " + field("hostname").toString());

    switch(this->pkgtype) {
    case Mobile:
        lines << "pkginstall powerdevil upower";
#if __cplusplus >= 201703L
        [[ fallthrough ]];
#endif
    case Standard:
        lines << "pkginstall adelie-base-posix firefox-esr libreoffice "
                 "thunderbird vlc kde x11";
        break;
    case Compact:
        lines << "pkginstall adelie-base netsurf featherpad lxqt-desktop "
                 "abiword gnumeric xorg-apps xorg-drivers xorg-server";
        break;
    case TextOnly:
        lines << "pkginstall adelie-base links tmux";
        break;
    case Custom:
        lines << ("pkginstall " + packages.join(" "));
        break;
    }

    char *root = encrypt_pw(field("rootpw").toString().toStdString().c_str());
    Q_ASSERT(root != nullptr);
    lines << QString("rootpw ") + root;
    delete[] root;

    /* XXX TODO someday we'll have language support */
    lines << "language en_GB.UTF-8";

    auto iterator = valid_keymaps.begin();
    std::advance(iterator, field("keymap").toInt());
    lines << ("keymap " + QString::fromStdString(*iterator));

#ifdef NON_LIBRE_FIRMWARE
    if(this->firmware) {
        lines << "firmware true";
    } else {
        lines << "firmware false";
    }
#endif  /* NON_LIBRE_FIRWMARE */

    lines << ("timezone " +
              dynamic_cast<DateTimePage *>(page(Page_DateTime))->selectedTimeZone());

    for(auto &acctWidget : dynamic_cast<AccountPage *>(page(Page_Accounts))->accountWidgets) {
        if(acctWidget->accountText().isEmpty()) break;

        char *userpw = encrypt_pw(acctWidget->passphraseText().toStdString().c_str());
        Q_ASSERT(userpw != nullptr);

        lines << ("username " + acctWidget->accountText());
        lines << ("useralias " + acctWidget->accountText() + " " +
                  acctWidget->personalText());
        lines << ("userpw " + acctWidget->accountText() + " " + userpw);
        delete[] userpw;
        if(acctWidget->isAdmin()) {
            lines << ("usergroups " + acctWidget->accountText() + " " +
                      "users,lp,audio,cdrom,cdrw,scanner,camera,video,games,usb,kvm,wheel");
        } else {
            lines << ("usergroups " + acctWidget->accountText() + " " +
                      "users,lp,audio,cdrom,cdrw,scanner,camera,video,games");
        }
    }

    return lines.join("\n");
}


#include <iostream>

void HorizonWizard::accept() {
    std::cout << toHScript().toStdString() << std::endl;
}
