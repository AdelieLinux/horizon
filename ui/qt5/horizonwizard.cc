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
#include <map>
#include <string>

#include "intropage.hh"
#include "networkingpage.hh"
#include "netsimplewifipage.hh"

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

    /* REQ: UI.Global.Back.Save */
    setOption(IndependentPages);
    /* REQ: UI.Language.Buttons, Iface.UI.StandardButtons */
    setOption(HaveHelpButton);
    setOption(NoBackButtonOnStartPage);

    setSizeGripEnabled(false);

    setPage(Page_Intro, new IntroPage);
    setPage(Page_Network, new NetworkingPage);

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
    QObject::disconnect(button(CancelButton), &QAbstractButton::clicked,
                        this, &QWizard::reject);
    QObject::connect(button(CancelButton), &QAbstractButton::clicked,
                     [=](bool) {
        QMessageBox cancel(QMessageBox::Question,
                           tr("Cancel Adélie Linux System Installation"),
                   #ifdef HAS_INSTALL_ENV
                           tr("Adélie Linux has not yet been set up on your computer.  Cancellation will reboot your computer.  Do you wish to cancel?"),
                   #else
                           tr("You have not yet completed the System Installation wizard.  No installfile will be generated.  Do you wish to cancel?"),
                   #endif
                           QMessageBox::Yes | QMessageBox::No,
                           this);
        cancel.setDefaultButton(QMessageBox::No);
        if(cancel.exec() == QMessageBox::Yes) {
            reject();
        }
    });

    /* REQ: Iface.UI.ButtonAccel */
    setButtonText(HelpButton, tr("Help (F1)"));
    setButtonText(CancelButton, tr("Cancel (F3)"));
    setButtonText(BackButton, tr("Back (F5)"));
    setButtonText(NextButton, tr("Next (F8)"));

    f1 = new QShortcut(Qt::Key_F1, this);
    connect(f1, &QShortcut::activated,
            button(HelpButton), &QAbstractButton::click);
    f1->setWhatsThis(tr("Activates the Help screen."));

    f3 = new QShortcut(Qt::Key_F3, this);
    connect(f3, &QShortcut::activated,
            button(CancelButton), &QAbstractButton::click);
    f3->setWhatsThis(tr("Prompts to cancel the installation."));

    f5 = new QShortcut(Qt::Key_F5, this);
    connect(f5, &QShortcut::activated,
            button(BackButton), &QAbstractButton::click);
    f5->setWhatsThis(tr("Goes back to the previous page."));

    f8 = new QShortcut(Qt::Key_F8, this);
    connect(f8, &QShortcut::activated,
            button(NextButton), &QAbstractButton::click);
    f8->setWhatsThis(tr("Goes forward to the next page."));
}
