/*
 * commitpage.cc - Implementation of the UI.Commit page
 * horizon-qt5, the Qt 5 user interface for
 * Project Horizon
 *
 * Copyright (c) 2019 Adélie Linux and contributors.  All rights reserved.
 * This code is licensed under the AGPL 3.0 license, as noted in the
 * LICENSE-code file in the root directory of this repository.
 *
 * SPDX-License-Identifier: AGPL-3.0-only
 */

#include "commitpage.hh"

#include "datetimepage.hh"
#include "util/keymaps.hh"
#include <algorithm>

#include <QLabel>
#include <QVariant>
#include <QVBoxLayout>

CommitPage::CommitPage(QWidget *parent) : HorizonWizardPage(parent) {
    setTitle(tr("Begin Installation"));
    loadWatermark("intro");

    QLabel *descLabel = new QLabel(tr(
        "We have gathered all the information needed to begin installing Adélie Linux to your computer.  "
        "Choose 'Install' when you are ready to begin the installation procedure."));
    descLabel->setWordWrap(true);

    choices = new QLabel;
    choices->setFrameStyle(QFrame::Panel | QFrame::Sunken);
    choices->setTextFormat(Qt::RichText);
    choices->setWordWrap(true);

    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(descLabel);
    layout->addSpacing(10);
    layout->addWidget(choices);

    setLayout(layout);
}

void CommitPage::initializePage() {
    QString diskString, netString, zoneString, softString;

    auto iterator = valid_keymaps.begin();
    Q_ASSERT(field("keymap").toUInt() <= valid_keymaps.size());
    std::advance(iterator, field("keymap").toUInt());

    if(horizonWizard()->auto_part) {
        if(horizonWizard()->erase) {
            diskString = QString{"Erase and Partition %1"}
                    .arg(QString::fromStdString(horizonWizard()->chosen_disk));
        } else {
            diskString = QString{"Install to Free Space on %1"}
                    .arg(QString::fromStdString(horizonWizard()->chosen_disk));
        }
    } else {
        diskString = "Custom Partitioning";
    }

    if(horizonWizard()->network &&
       horizonWizard()->interfaces.size() > 1) {
        QString iface = QString::fromStdString(horizonWizard()->chosen_auto_iface);
        netString = tr("Enabled (via %1)").arg(iface);
    } else {
        netString = horizonWizard()->network ? tr("Enabled") : tr("Disabled");
    }

    zoneString = dynamic_cast<DateTimePage *>(
                horizonWizard()->page(HorizonWizard::Page_DateTime)
                )->selectedTimeZone();

    switch(horizonWizard()->pkgtype) {
    case HorizonWizard::Standard:
        softString = tr("Standard");
        break;
    case HorizonWizard::Mobile:
        softString = tr("Mobile");
        break;
    case HorizonWizard::Compact:
        softString = tr("Compact");
        break;
    case HorizonWizard::TextOnly:
        softString = tr("Text-Only");
        break;
    case HorizonWizard::Custom:
        /* XXX TODO maybe show packages selected?  too lengthy? */
        softString = tr("Custom");
        break;
    }

    choices->setText(tr("<br>"
            "<p><b>Disk Layout:</b> %1</p>\n"
            "<p><b>Keyboard Layout</b>: %2</p>\n"
            "<p><b>Networking</b>: %3</p>\n"
            "<p><b>Time Zone</b>: %4</p>\n"
            "<p><b>Software Selection</b>: %5</p>\n"
            "<p><b>Hostname</b>: %6</p>\n"
            "<p><b>Root Passphrase</b>: <i>[saved]</i></p>\n"
            "<br>")
            .arg(diskString)
            .arg(QString::fromStdString(*iterator))
            .arg(netString)
            .arg(zoneString)
            .arg(softString)
            .arg(field("hostname").toString()));
}
