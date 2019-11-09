/*
 * intropage.cc - Implementation of the UI.Intro page
 * horizon-qt5, the Qt 5 user interface for
 * Project Horizon
 *
 * Copyright (c) 2019 Adélie Linux and contributors.  All rights reserved.
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
                tr("<p>"
                   "Horizon will guide you through creation of a basic <code>installfile</code> for installing Adélie Linux on another computer."
                   "<p>"
                   "<b>IMPORTANT:</b> You may be allowed to specify an invalid or non-bootable disk layout or network configuration.  "
                   "For best results, run System Installation directly on the computer you wish to run Adélie Linux."
                   "<p>"
                   "For more information about the <code>installfile</code> format and syntax, see the "
                   "<a href='https://help.adelielinux.org/html/install/'>"
                   "Adélie Linux Installation Guide</a> on the Internet."));
    descLabel->setOpenExternalLinks(true);
    descLabel->setTextFormat(Qt::RichText);
#else  /* HAS_INSTALL_ENV */
    QMenu *toolMenu;
    toolButton = new QPushButton(tr("Launch &Tool..."), this);
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
                tr("Thank you for choosing the Adélie Linux operating system.  "
                   "This installation process will only take about 10-15 minutes of your time.  "
                   "When you're done, your computer will be up and running with the reliable, secure, libre Adélie Linux.\n\n"

                   "When you're ready to answer a few questions, you can get started by choosing Next or pressing F8.  "
                   "If you'd like more information about the installation procedure, choose Help or press F1 at any time.\n\n"

                   "The installation procedure will not change anything on your computer until the final step.  "
                   "You can safely cancel at any time, with no change to your computer, by choosing Cancel or pressing F3.\n\n"

                   "If you are unable to use a mouse, you may press the Tab key to cycle between the available inputs.  "
                   "The currently selected input will be highlighted."));
#endif  /* !HAS_INSTALL_ENV */
    descLabel->setWordWrap(true);

    layout = new QVBoxLayout;
    layout->addWidget(descLabel);
#ifdef HAS_INSTALL_ENV
    layout->addStretch();
    layout->addWidget(toolButton);
#endif  /* HAS_INSTALL_ENV */
    setLayout(layout);
}
