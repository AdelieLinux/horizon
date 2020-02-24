/*
 * intropage.cc - Implementation of the UI.Intro page
 * horizon-qt5, the Qt 5 user interface for
 * Project Horizon
 *
 * Copyright (c) 2019-2020 Adélie Linux and contributors.  All rights reserved.
 * This code is licensed under the AGPL 3.0 license, as noted in the
 * LICENSE-code file in the root directory of this repository.
 *
 * SPDX-License-Identifier: AGPL-3.0-only
 */

#include "intropage.hh"

#include <QLabel>
#ifdef HAS_INSTALL_ENV
#include <QMenu>
#include <QProcess>
#endif  /* HAS_INSTALL_ENV */
#include <QVBoxLayout>

IntroPage::IntroPage(QWidget *parent) : HorizonWizardPage(parent) {
    QLabel *descLabel;
    QVBoxLayout *layout;

    loadWatermark("intro");
    setTitle(tr("Welcome to Adélie Linux"));

#ifndef HAS_INSTALL_ENV
    descLabel = new QLabel(
                tr("<p>Thank you for choosing the reliable, secure, libre Adélie Linux operating environment.  "
                   "This process will take about 10-15 minutes.</p>"
                   "<p>Using this wizard, you will create a basic <code>installfile</code>, which you can use to install Adélie Linux on a computer over the network or using custom installation media.</p>"
                   "<p><strong>IMPORTANT:</strong> You may be allowed to specify an invalid or non-bootable disk layout or network configuration.  "
                   "For best results, run System Installation directly on the computer you wish to run Adélie Linux.</p>"
                   "<p>For more information about the installfile format and syntax, see the <a href=\"https://help.adelielinux.org/html/install/\">Adélie Linux Installation Handbook</a>.</p>"));
    descLabel->setOpenExternalLinks(true);
    descLabel->setTextFormat(Qt::RichText);
#else  /* HAS_INSTALL_ENV */
    QMenu *toolMenu;
    toolButton = new QPushButton(tr("Launch &Tool..."), this);
    toolButton->setWhatsThis(tr("Displays a menu of utilities that you can launch from the Installation Environment."));
    toolMenu = new QMenu("&Tools", toolButton);
    connect(toolMenu->addAction("&Terminal"), &QAction::triggered, [=](void) {
        QProcess p;
        p.execute("xterm", {"-fa", "Liberation Mono", "-fs", "12"});
    });
    connect(toolMenu->addAction("&Partition Editor"), &QAction::triggered, [=](void) {
        QProcess p;
        p.execute("partitionmanager");
    });
    /*connect(toolMenu->addAction("&Web Browser"), &QAction::triggered, [=](void){
        QProcess p;
        p.execute("otter-browser");
    });*/
    toolButton->setMenu(toolMenu);

    descLabel = new QLabel(
                tr("Thank you for choosing the reliable, secure, libre Adélie Linux operating environment.  "
                   "Installation will take about 15-30 minutes.\n\n"

                   "When you're ready to answer a few questions, choose Continue or press the F8 key.  "
                   "To learn more about the installation procedure, choose Help or press the F1 key at any time.\n\n"

                   "System Installation will not modify any data or settings on your computer until the final step.  "
                   "To exit without modifying your computer, choose Exit or press the F3 key.\n\n"

                   "If you cannot use a mouse, you may press the Tab key to cycle between input fields.  "
                   "The currently selected field will be highlighted."));
#endif  /* !HAS_INSTALL_ENV */
    descLabel->setWordWrap(true);

    layout = new QVBoxLayout;
    layout->addWidget(descLabel);
#ifdef HAS_INSTALL_ENV
    layout->addStretch();
    layout->addWidget(toolButton, 0, Qt::AlignCenter);
#endif  /* HAS_INSTALL_ENV */
    setLayout(layout);
}
