#include "horizonwizard.hh"
#include "horizonhelpwindow.hh"

#include <QDebug>
#include <QFile>
#include <QMessageBox>
#include <map>
#include <string>

#include "intropage.hh"
#include "networkingpage.hh"
#include "netsimplewifipage.hh"

using std::map;
using std::string;

static map<int, string> help_id_map = {
    {HorizonWizard::Page_Intro, "intro"},
    {HorizonWizard::Page_Input, "input"},
    {HorizonWizard::Page_Partition, "partition"},
#ifdef NON_LIBRE_FIRMWARE
    {HorizonWizard::Page_Firmware, "firmware"},
#endif  /* NON_LIBRE_FIRMWARE */
    {HorizonWizard::Page_Network, "network-start"},
    {HorizonWizard::Page_Network_Iface, "network-iface"},
    {HorizonWizard::Page_Network_Wireless, "network-wifi"},
    {HorizonWizard::Page_Network_DHCP, "none"},
    {HorizonWizard::Page_Network_Portal, "network-portal"},
    {HorizonWizard::Page_Network_Manual, "network-manual"},
    {HorizonWizard::Page_DateTime, "datetime"},
    {HorizonWizard::Page_Hostname, "hostname"},
    {HorizonWizard::Page_PkgSimple, "packages-simple"},
    {HorizonWizard::Page_PkgCustom, "packages-custom"},
    {HorizonWizard::Page_Boot, "startup"},
    {HorizonWizard::Page_Root, "rootpw"},
    {HorizonWizard::Page_Accounts, "accounts"},
#ifndef HAS_INSTALL_ENV
    {HorizonWizard::Page_Write, "writeout"},
#else  /* HAS_INSTALL_ENV */
    {HorizonWizard::Page_Commit, "commit"}
#endif  /* !HAS_INSTALL_ENV */
};

HorizonWizard::HorizonWizard(QWidget *parent) : QWizard(parent) {
    setWindowTitle(tr("Adélie Linux System Installation"));

    setFixedSize(QSize(650, 450));

    setOption(DisabledBackButtonOnLastPage);
    setOption(HaveHelpButton);
    setOption(NoCancelButton);

    setSizeGripEnabled(false);

    setPage(Page_Intro, new IntroPage);
    setPage(Page_Network, new NetworkingPage);

    QObject::connect(this, static_cast<void (QWizard:: *)(void)>(&QWizard::helpRequested),
                     [=](void) {
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
        string helppath = ":/wizard_help/resources/" +
                          help_id_map.at(currentId()) + "-help.txt";
        QFile helpfile(helppath.c_str());
        helpfile.open(QFile::ReadOnly);
        HorizonHelpWindow help(&helpfile, this);
        help.exec();
    });
}
