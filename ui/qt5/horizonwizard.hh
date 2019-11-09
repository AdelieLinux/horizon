#ifndef HORIZONWIZARD_HH
#define HORIZONWIZARD_HH

#include <QShortcut>
#include <QWizard>
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
        Page_Network,           /* network type selection (DHCP/static) */
        Page_Network_Iface,     /* network interface selection */
        Page_Network_Wireless,  /* wireless */
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

    HorizonWizard(QWidget *parent = nullptr);
    QShortcut *f1, *f3, *f5, *f8;
};

#endif  /* !HORIZONWIZARD_HH */
