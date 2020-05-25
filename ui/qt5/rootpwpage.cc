/*
 * rootpwpage.cc - Implementation of the UI.Accounts.RootPW page
 * horizon-qt5, the Qt 5 user interface for
 * Project Horizon
 *
 * Copyright (c) 2019 Adélie Linux and contributors.  All rights reserved.
 * This code is licensed under the AGPL 3.0 license, as noted in the
 * LICENSE-code file in the root directory of this repository.
 *
 * SPDX-License-Identifier: AGPL-3.0-only
 */

#include "rootpwpage.hh"

#include <QAction>
#include <QFormLayout>
#include <QLabel>
#include <QLineEdit>
#include <QVBoxLayout>

RootPassphrasePage::RootPassphrasePage(QWidget *parent)
    : HorizonWizardPage(parent) {
    setTitle(tr("Set Root Passphrase"));
    loadWatermark("acct");

    QLabel *descLabel = new QLabel(tr(
        "You need to set a root passphrase for this computer.\n\n"
        "The root passphrase is used for system administration and recovery.  "
        "Be sure to write it down and keep it in a safe place.\n\n"
        "The root passphrase must be at least 8 characters long, and can contain any mixture of letters, numbers, and symbols."));
    descLabel->setWordWrap(true);

    rootPW = new QLineEdit;
    rootPW->setEchoMode(QLineEdit::Password);
    rootPW->setWhatsThis(tr("Enter your desired root passphrase here."));
    connect(rootPW, &QLineEdit::textChanged,
            this, &RootPassphrasePage::completeChanged);
    QAction *togglePass = rootPW->addAction(QIcon::fromTheme("visibility"),
                                            QLineEdit::TrailingPosition);
    togglePass->setToolTip(tr("Show the passphrase"));
    togglePass->setData(true);
    connect(togglePass, &QAction::triggered,
            [=](void) {
        if(togglePass->data().toBool() == true) {
            togglePass->setData(false);
            togglePass->setIcon(QIcon::fromTheme("hint"));
            togglePass->setToolTip(tr("Hide the passphrase"));
            rootPW->setEchoMode(QLineEdit::Normal);
        } else {
            togglePass->setData(true);
            togglePass->setIcon(QIcon::fromTheme("visibility"));
            togglePass->setToolTip(tr("Show the passphrase"));
            rootPW->setEchoMode(QLineEdit::Password);
        }
    });
    registerField("rootpw", rootPW);
    confirmPW = new QLineEdit;
    confirmPW->setEchoMode(QLineEdit::Password);
    confirmPW->setWhatsThis(tr(
        "Confirm your desired root passphrase by typing it again here."));
    connect(confirmPW, &QLineEdit::textChanged,
            this, &RootPassphrasePage::completeChanged);
    QAction *toggleConfPass = confirmPW->addAction(
                QIcon::fromTheme("visibility"), QLineEdit::TrailingPosition);
    toggleConfPass->setToolTip(tr("Show the passphrase"));
    toggleConfPass->setData(true);
    connect(toggleConfPass, &QAction::triggered,
            [=](void) {
        if(toggleConfPass->data().toBool() == true) {
            toggleConfPass->setData(false);
            toggleConfPass->setIcon(QIcon::fromTheme("hint"));
            toggleConfPass->setToolTip(tr("Hide the passphrase"));
            confirmPW->setEchoMode(QLineEdit::Normal);
        } else {
            toggleConfPass->setData(true);
            toggleConfPass->setIcon(QIcon::fromTheme("visibility"));
            toggleConfPass->setToolTip(tr("Show the passphrase"));
            confirmPW->setEchoMode(QLineEdit::Password);
        }
    });
    QFormLayout *pwForm = new QFormLayout;
    pwForm->addRow(tr("&Passphrase:"), rootPW);
    pwForm->addRow(tr("&Confirm Passphrase:"), confirmPW);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(descLabel);
    mainLayout->addStretch();
    mainLayout->addLayout(pwForm);
    mainLayout->addStretch();
    setLayout(mainLayout);
}

bool RootPassphrasePage::isComplete() const {
    return (rootPW->text().size() >= 8 &&
            rootPW->text() == confirmPW->text());
}
