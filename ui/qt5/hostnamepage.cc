/*
 * hostnamepage.cc - Implementation of the UI.SysMeta.Hostname page
 * horizon-qt5, the Qt 5 user interface for
 * Project Horizon
 *
 * Copyright (c) 2019 Adélie Linux and contributors.  All rights reserved.
 * This code is licensed under the AGPL 3.0 license, as noted in the
 * LICENSE-code file in the root directory of this repository.
 *
 * SPDX-License-Identifier: AGPL-3.0-only
 */

#include "hostnamepage.hh"

#include <QLabel>
#include <QRegExpValidator>
#include <QVBoxLayout>

HostnamePage::HostnamePage(QWidget *parent) : HorizonWizardPage(parent) {
    QLabel *descLabel;
    QRegExpValidator *hostnameValidator = new QRegExpValidator(this);
    hostnameValidator->setRegExp(QRegExp("[A-Za-z][A-Za-z0-9-_.]+"));
    QVBoxLayout *layout;

    setTitle(tr("Computer Name"));
    loadWatermark("intro");

    descLabel = new QLabel(tr("<p>You need to provide a name for your computer.  "
                              "You may accept the default, or choose your own.  "
                              "If your computer participates in a network, make sure that the name is unique.</p>"

                              "<p>Computer names may contain letters, numbers, dashes (-), and underscores (_).  For example, <i>Living-Room-PC</i>.</p>"

                              "<p>You may also specify your computer's domain name if necessary (not common).  For example, <i>desktop1.my.network</i>.</p>"), this);
    descLabel->setTextFormat(Qt::RichText);
    descLabel->setWordWrap(true);
    hostnameEdit = new QLineEdit(this);
    hostnameEdit->setPlaceholderText(tr("Computer Name"));
    hostnameEdit->setValidator(hostnameValidator);
    hostnameEdit->setWhatsThis(tr("Enter the name of your computer here."));

    registerField("hostname*", hostnameEdit);

    layout = new QVBoxLayout(this);
    layout->addWidget(descLabel);
    layout->addStretch();
    layout->addWidget(hostnameEdit);
    layout->addStretch();
    setLayout(layout);
}

void HostnamePage::initializePage() {
#ifndef HAS_INSTALL_ENV
    hostnameEdit->setText("Adelie");
#else  /* HAS_INSTALL_ENV */
    QString name("Adelie");

    if(horizonWizard()->interfaces.empty()) {
        name += "-Banana";
    } else {
        QString mac = horizonWizard()->interfaces.begin()->second.mac;
        QStringList macparts = mac.split(":");
        name += "-" + macparts[3] + macparts[4] + macparts[5];
    }

    hostnameEdit->setText(name);
#endif  /* !HAS_INSTALL_ENV */
}
