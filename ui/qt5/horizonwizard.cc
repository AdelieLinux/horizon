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
#   include <QApplication>
#   include <QProcess>
#   include <libudev.h>
#   include <net/if.h>      /* ifreq */
#   include "commitpage.hh"
extern "C" {
#   include <skalibs/tai.h> /* STAMP */
}
#   include <sys/ioctl.h>   /* ioctl */
#   include <unistd.h>      /* close */
#else
#   include <QFileDialog>
#   include "writeoutpage.hh"
#endif  /* HAS_INSTALL_ENV */

#include "intropage.hh"
#include "inputpage.hh"
#ifdef NON_LIBRE_FIRMWARE
#include "firmwarepage.hh"
#endif  /* NON_LIBRE_FIRMWARE */
#include "partitionpage.hh"
#include "partitiondiskpage.hh"
#include "partitionchoicepage.hh"
#include "partitionmanualpage.hh"
#include "partitionmountpage.hh"
#include "networkingpage.hh"
#include "networkifacepage.hh"
#include "netsimplewifipage.hh"
#include "netmanualpage.hh"
#include "netdhcppage.hh"
#include "datetimepage.hh"
#include "hostnamepage.hh"
#include "pkgsimple.hh"
#include "pkgcustom.hh"
#include "pkgdefaults.hh"
#include "bootpage.hh"
#include "rootpwpage.hh"
#include "accountpage.hh"

#include "util/keymaps.hh"

static std::map<int, std::string> help_id_map = {
    {HorizonWizard::Page_Intro, "intro"},
    {HorizonWizard::Page_Input, "input"},
#ifdef NON_LIBRE_FIRMWARE
    {HorizonWizard::Page_Firmware, "firmware"},
#endif  /* NON_LIBRE_FIRMWARE */
#ifdef HAS_INSTALL_ENV
    {HorizonWizard::Page_Partition, "partition"},
#endif  /* HAS_INSTALL_ENV */
    {HorizonWizard::Page_PartitionDisk, "partition-disk"},
    {HorizonWizard::Page_PartitionChoose, "partition-manipulation"},
    {HorizonWizard::Page_PartitionManual, "partition-manual"},
    {HorizonWizard::Page_PartitionMount, "partition-mountpoints"},
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


#if defined(HAS_INSTALL_ENV) && defined(__powerpc64__)
HorizonWizard::Subarch determinePowerPCPlatform() {
    if(!QFile::exists("/tmp/horizon-platform")) {
        QProcess p;
        p.execute("horizon-ppc64-detect");
    }

    QFile platform;
    platform.setFileName("/tmp/horizon-platform");
    if(!platform.open(QFile::ReadOnly)) {
        qDebug() << "can't read PowerPC platform type!";
        return HorizonWizard::ppc64_pSeries;
    }

    QByteArray result = platform.readAll();
    platform.close();

    if(result.startsWith("PowerMac")) {
        return HorizonWizard::ppc64_PowerMac;
    } else if(result.startsWith("PowerNV")) {
        return HorizonWizard::ppc64_PowerNV;
    } else {
        return HorizonWizard::ppc64_pSeries;
}
}
#endif


HorizonWizard::HorizonWizard(QWidget *parent) : QWizard(parent) {
    setWindowTitle(tr("Adélie Linux System Installation"));

    setFixedSize(QSize(650, 450));

    mirror_domain = "distfiles.adelielinux.org";
    version = "stable";
    subarch = NoSubarch;
    /* handwavy-future:
     * Determine which platform kernel is being used, if any (-power8 etc)
     * Determine hardware requirements (easy or mainline)
     */
    grub = true;
    kernel = "easy-kernel";
    binsh = Dash;
    sbininit = S6;
    eudev = true;
    ipv4.use = false;
    ipv6.use = false;

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
#ifdef HAS_INSTALL_ENV
    setPage(Page_Partition, new PartitionPage);
#endif  /* HAS_INSTALL_ENV */
    setPage(Page_PartitionDisk, new PartitionDiskPage);
    setPage(Page_PartitionChoose, new PartitionChoicePage);
    setPage(Page_PartitionManual, new PartitionManualPage);
    setPage(Page_PartitionMount, new PartitionMountPage);
    setPage(Page_Network, new NetworkingPage);
    setPage(Page_Network_Iface, new NetworkIfacePage);
    setPage(Page_Network_Wireless, new NetworkSimpleWirelessPage);
    setPage(Page_Network_DHCP, new NetDHCPPage);
    setPage(Page_Network_Manual, new NetManualPage);
    setPage(Page_DateTime, new DateTimePage);
    setPage(Page_Hostname, new HostnamePage);
    setPage(Page_PkgSimple, new PkgSimplePage);
    setPage(Page_PkgCustom, new PkgCustomPage);
    setPage(Page_PkgCustomDefault, new PkgDefaultsPage);
    setPage(Page_Boot, new BootPage);
    setPage(Page_Root, new RootPassphrasePage);
    setPage(Page_Accounts, new AccountPage);
#ifndef HAS_INSTALL_ENV
    setPage(Page_Write, new WriteoutPage);
#else  /* HAS_INSTALL_ENV */
    setPage(Page_Commit, new CommitPage);
#endif  /* !HAS_INSTALL_ENV */

    QObject::connect(this, &QWizard::helpRequested, [=](void) {
        if(help_id_map.find(currentId()) == help_id_map.end()) {
            qDebug() << "no help available for " << currentId();
            QMessageBox nohelp(QMessageBox::Warning,
                               tr("No Help Available"),
                               tr("Help is not available for the current page."
                                  "  Consult the Installation Handbook for "
                                  "more information."),
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

    /* REQ: Iface.UI.ButtonAccel */
    setButtonText(HelpButton, tr("&Help (F1)"));
    setButtonText(CancelButton, tr("E&xit (F3)"));
    setButtonText(BackButton, tr("< &Back (F6)"));
    setButtonText(NextButton, tr("Co&ntinue > (F8)"));
#ifdef HAS_INSTALL_ENV
    setButtonText(FinishButton, tr("&Install (F10)"));
#else
    setButtonText(FinishButton, tr("&Save (F10)"));
#endif  /* HAS_INSTALL_ENV */

    button(HelpButton)->setWhatsThis(tr("Activates the Help screen."));
    button(CancelButton)->setWhatsThis(tr("Prompts to close System Installation."));
    button(BackButton)->setWhatsThis(tr("Goes back to the previous page of System Installation."));
    button(NextButton)->setWhatsThis(tr("Goes forward to the next page of System Installation."));
    button(FinishButton)->setWhatsThis(tr("Completes the information gathering phase of System Installation."));

    f1 = new QShortcut(Qt::Key_F1, this);
    connect(f1, &QShortcut::activated,
            button(HelpButton), &QAbstractButton::click);
    f1->setWhatsThis(tr("Activates the Help screen."));

    f3 = new QShortcut(Qt::Key_F3, this);
    connect(f3, &QShortcut::activated,
            button(CancelButton), &QAbstractButton::click);
    f3->setWhatsThis(tr("Prompts to close System Installation."));

    f6 = new QShortcut(Qt::Key_F6, this);
    connect(f6, &QShortcut::activated,
            button(BackButton), &QAbstractButton::click);
    f6->setWhatsThis(tr("Goes back to the previous page of System Installation."));

    f8 = new QShortcut(Qt::Key_F8, this);
    connect(f8, &QShortcut::activated,
            button(NextButton), &QAbstractButton::click);
    f8->setWhatsThis(tr("Goes forward to the next page of System Installation."));

    f10 = new QShortcut(Qt::Key_F10, this);
    connect(f10, &QShortcut::activated,
            button(FinishButton), &QAbstractButton::click);
    f10->setWhatsThis(tr("Completes the information gathering phase of System Installation."));

#ifdef HAS_INSTALL_ENV
    interfaces = probe_ifaces();
    tain_now_set_stopwatch_g();

#   if defined(__powerpc64__)
    arch = ppc64;
    subarch = determinePowerPCPlatform();
#   elif defined(__powerpc__)
    arch = ppc;
#   elif defined(__aarch64__)
    arch = aarch64;
#   elif defined(__arm__)
    arch = armv7;
#   elif defined(__i386__)
    arch = pmmx;
#   elif defined(__x86_64__)
    arch = x86_64;
#   else
#       error You are attempting to compile the Installation Environment UI \
on an unknown architecture.  Please add a definition to horizonwizard.hh, a \
setting in this preprocessor block, and a case statement for the automatic \
partitioner to continue.  You may also wish to add an option to archpage.cc.
#   endif
#else  /* !HAS_INSTALL_ENV */
    arch = UnknownCPU;
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


/*! Determine the node name of a given partition. */
QString nameForPartitionOnDisk(const std::string &dev, int part) {
    const std::string maybe_p(::isdigit(dev[dev.size() - 1]) ? "p" : "");
    std::string raw_root = dev + maybe_p + std::to_string(part);
    return QString::fromStdString(raw_root);
}


/*! Determine the correct disk label based on the target platform. */
QStringList eraseDiskForArch(const std::string &raw_disk,
                             HorizonWizard::Arch arch,
                             HorizonWizard::Subarch subarch) {
    QString disk = QString::fromStdString(raw_disk);

    switch(arch) {
    case HorizonWizard::aarch64:/* 64-bit ARM mostly uses GPT */
    case HorizonWizard::x86_64: /* 64-bit Intel uses GPT */
        return {QString{"disklabel %1 gpt"}.arg(disk)};
    case HorizonWizard::ppc:    /* 32-bit PowerPC: we only support Power Mac */
        return {QString{"disklabel %1 apm"}.arg(disk)};
    case HorizonWizard::ppc64:  /* Complicated */
        switch(subarch) {
        case HorizonWizard::ppc64_PowerMac:
            return {QString{"disklabel %1 apm"}.arg(disk)};
        case HorizonWizard::ppc64_PowerNV:
            return {QString{"disklabel %1 gpt"}.arg(disk)};
        case HorizonWizard::ppc64_pSeries:
        default:
            return {QString{"disklabel %1 mbr"}.arg(disk)};
        }
    case HorizonWizard::armv7:  /* your guess is as good as mine */
    case HorizonWizard::pmmx:   /* 32-bit Intel requires MBR */
    case HorizonWizard::UnknownCPU: /* safe enough as a fallback */
    default:
        return {QString{"disklabel %1 mbr"}.arg(disk)};
    }
}


/*! Determine the correct boot disk layout based on the target platform. */
QStringList bootForArch(const std::string &raw_disk, HorizonWizard::Arch arch,
                        HorizonWizard::Subarch subarch, int *start) {
    QString disk = QString::fromStdString(raw_disk);

    switch(arch) {
    case HorizonWizard::aarch64:/* 64-bit ARM: assume UEFI */
        return {
            QString{"partition %1 %2 256M esp"}.arg(disk).arg(*start),
            QString{"fs %1 fat32"}.arg(nameForPartitionOnDisk(raw_disk, *start)),
            QString{"mount %1 /boot/efi"}.arg(nameForPartitionOnDisk(raw_disk, (*start)++))
        };
    case HorizonWizard::x86_64: /* 64-bit Intel: support UEFI and BIOS */
        return {
            QString{"partition %1 %2 1M bios"}.arg(disk).arg((*start)++),
            QString{"partition %1 %2 256M esp"}.arg(disk).arg(*start),
            QString{"fs %1 fat32"}.arg(nameForPartitionOnDisk(raw_disk, *start)),
            QString{"mount %1 /boot/efi"}.arg(nameForPartitionOnDisk(raw_disk, (*start)++))
        };
    case HorizonWizard::ppc:    /* 32-bit PowerPC: we only support Power Mac */
        return {
            QString{"partition %1 %2 16M boot"}.arg(disk).arg(*start),
            QString{"fs %1 hfs+"}.arg(nameForPartitionOnDisk(raw_disk, *start)),
            QString{"mount %1 /boot/grub"}.arg(nameForPartitionOnDisk(raw_disk, (*start)++))
        };
    case HorizonWizard::ppc64:  /* Complicated */
        switch(subarch) {
        case HorizonWizard::ppc64_PowerMac: /* as ppc32 */
            return {
                QString{"partition %1 %2 16M boot"}.arg(disk).arg(*start),
                QString{"fs %1 hfs+"}.arg(nameForPartitionOnDisk(raw_disk, *start)),
                QString{"mount %1 /boot/grub"}.arg(nameForPartitionOnDisk(raw_disk, (*start)++))
            };
        case HorizonWizard::ppc64_PowerNV:  /* doesn't need a separate /boot */
            return {};
        case HorizonWizard::ppc64_pSeries:  /* PReP boot partition */
        default:
            return {
                QString{"partition %1 %2 10M prep"}.arg(disk).arg((*start)++),
            };
        }
    case HorizonWizard::armv7:
    case HorizonWizard::pmmx:   /* 32-bit Intel, bog standard GRUB */
    case HorizonWizard::UnknownCPU: /* safe enough as a fallback */
    default:
        return {
            QString{"partition %1 %2 256M boot"}.arg(disk).arg(*start),
            QString{"fs %1 ext2"}.arg(nameForPartitionOnDisk(raw_disk, *start)),
            QString{"mount %1 /boot"}.arg(nameForPartitionOnDisk(raw_disk, (*start)++))
        };
    }
}


QString HorizonWizard::toHScript() {
    QStringList lines;

    if(this->network) {
        lines << "network true";
        lines << "pkginstall iproute2";

        if(this->net_dhcp) {
            lines << QString::fromStdString("netaddress " +
                                            this->chosen_auto_iface + " dhcp");
        } else {
            Q_ASSERT(this->ipv6.use || this->ipv4.use);

            if(this->ipv6.use) {
                QString addrL = QString::fromStdString("netaddress " +
                                                       this->chosen_auto_iface);
                addrL += " static " + this->ipv6.address + " " + this->ipv6.mask;
                if(this->ipv6.gateway.size() > 0) {
                    addrL += " " + this->ipv6.gateway;
                }
                lines << addrL;
                lines << ("nameserver " + this->ipv6.dns);
            }
            if(this->ipv4.use) {
                QString addrL = QString::fromStdString("netaddress " +
                                                       this->chosen_auto_iface);
                addrL += " static " + this->ipv4.address + " " + this->ipv4.mask;
                if(this->ipv4.gateway.size() > 0) {
                    addrL += " " + this->ipv4.gateway;
                }
                lines << addrL;
                lines << ("nameserver " + this->ipv4.dns);
            }
        }
    } else {
        lines << "network false";
    }

    lines << ("hostname " + field("hostname").toString());

    switch(arch) {
    case aarch64:
        lines << "arch aarch64";
        break;
    case armv7:
        lines << "arch armv7";
        break;
    case pmmx:
        lines << "arch pmmx";
        break;
    case ppc:
        lines << "arch ppc";
        break;
    case ppc64:
        lines << "arch ppc64";
        break;
    case x86_64:
        lines << "arch x86_64";
        break;
    case UnknownCPU:
        /* no arch line.  hopefully it's run on the target. */
        break;
    }

    part_lines << (dynamic_cast<PartitionMountPage *>(page(Page_PartitionMount)))->mountLines();

    if(chosen_disk.empty()) {
        lines << part_lines;
    } else if(!auto_part) {
        lines << part_lines;
    } else {
        /* We have a chosen disk, and will be automatically partitioning it. */
        int start = 1;

        if(erase) {
            lines << eraseDiskForArch(chosen_disk, arch, subarch);
        } else {
#ifdef HAS_INSTALL_ENV
            Disk *disk = nullptr;
            for(auto &d_iter : disks) {
                if(d_iter.node() == chosen_disk) {
                    disk = &d_iter;
                    break;
                }
            }
            Q_ASSERT(disk != nullptr);
            start = disk->partitions().size() + 1;
#else  /* !HAS_INSTALL_ENV */
            Q_ASSERT(false);
#endif  /* HAS_INSTALL_ENV */
        }

        if(this->grub) {
            lines << bootForArch(chosen_disk, arch, subarch, &start);
        }

        /* Create the root partition */
        lines << QString{"partition %1 %2 fill"}
                 .arg(QString::fromStdString(chosen_disk)).arg(start);
        QString root_part = nameForPartitionOnDisk(chosen_disk, start);
        lines << QString{"fs %1 ext4"}.arg(root_part);
        lines << QString{"mount %1 /"}.arg(root_part);
    }

    switch(this->pkgtype) {
    case Mobile:
        lines << "pkginstall pm-utils pm-quirks powerdevil upower";
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
        lines << "pkginstall adelie-base-posix";
        if(!packages.empty()) lines << ("pkginstall " + packages.join(" "));
        break;
    }

    if(this->grub) {
        lines << "pkginstall grub";
    }

    switch(this->binsh) {
    case Dash:
        lines << "pkginstall dash-binsh";
        break;
    case Bash:
        lines << "pkginstall bash-binsh";
        break;
    }

    switch(this->sbininit) {
    case S6:
        lines << "pkginstall s6-linux-init";
        break;
    case SysVInit:
        lines << "pkginstall sysvinit";
        break;
    }

    if(this->eudev) {
        lines << "pkginstall eudev";
    } else {
        lines << "pkginstall mdevd";
    }

    lines << "pkginstall sysklogd";

    lines << ("pkginstall " + QString::fromStdString(this->kernel) + " " +
              QString::fromStdString(this->kernel) + "-modules");

    char *root = encrypt_pw(field("rootpw").toString().toStdString().c_str());
    Q_ASSERT(root != nullptr);
    lines << QString("rootpw ") + root;
    delete[] root;

    /* handwavy-future: When we have language support, set it here */
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
                      "users,lp,audio,cdrom,cdrw,video,games,usb,kvm,wheel");
        } else {
            lines << ("usergroups " + acctWidget->accountText() + " " +
                      "users,lp,audio,cdrom,cdrw,video,games");
        }
        if(!acctWidget->avatarPath().isEmpty()) {
            lines << ("usericon " + acctWidget->accountText() + " " +
                      acctWidget->avatarPath());
        }
    }

    return lines.join("\n");
}


void HorizonWizard::accept() {
    QFile file;
#ifdef HAS_INSTALL_ENV
    file.setFileName("/etc/horizon/installfile");
#else  /* !HAS_INSTALL_ENV */
    QFileDialog fileChooser(this);
    fileChooser.setAcceptMode(QFileDialog::AcceptSave);
    fileChooser.setNameFilter(tr("Installation Scripts (installfile)"));
    fileChooser.setViewMode(QFileDialog::List);

    if(fileChooser.exec()) {
        file.setFileName(fileChooser.selectedFiles().at(0));
    } else {
        return;
    }
#endif  /* HAS_INSTALL_ENV */
    if(!file.open(QFile::WriteOnly)) {
        QMessageBox::critical(this, tr("Couldn't Save Script"),
                              tr("An issue occurred while saving the installation script: %1").arg(file.errorString()));
        return;
    }
    file.write(toHScript().toUtf8());
    file.close();

    done(QDialog::Accepted);
}

void HorizonWizard::reject() {
    /* REQ: UI.Global.Cancel.Confirm */
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
#ifdef HAS_INSTALL_ENV
        qApp->exit(1);
#else  /* !HAS_INSTALL_ENV */
        done(QDialog::Rejected);
#endif  /* HAS_INSTALL_ENV */
    } else {
        return;
    }
}
