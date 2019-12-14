/*
 * useraccountwidget.cc - Implementation of a widget for managing user accounts
 * horizon-qt5, the Qt 5 user interface for
 * Project Horizon
 *
 * Copyright (c) 2019 Adélie Linux and contributors.  All rights reserved.
 * This code is licensed under the AGPL 3.0 license, as noted in the
 * LICENSE-code file in the root directory of this repository.
 *
 * SPDX-License-Identifier: AGPL-3.0-only
 */

#include "useraccountwidget.hh"

#include <QAction>
#include <QHBoxLayout>
#include <QMenu>
#include <QVBoxLayout>

UserAccountWidget::UserAccountWidget(QWidget *parent)
    : QWidget(parent), acctEverTouched(false) {
    QHBoxLayout *nameLayout = new QHBoxLayout;

    personalName = new QLineEdit;
    QFont personalFont = personalName->font();
    personalFont.setPointSize(personalFont.pointSize() + 2);
    personalName->setFont(personalFont);
    personalName->setPlaceholderText(tr("Personal Name"));
    personalName->setToolTip(tr("Used to address this user"));
    personalName->setWhatsThis(tr("This name will be used to address this user.  It may be their real name, or a nickname."));
    nameLayout->addWidget(personalName, 20);

    accountName = new QLineEdit;
    accountName->setMaxLength(32);
    accountName->setPlaceholderText(tr("Username"));
    accountName->setToolTip(tr("Used to identify this user; letters and numbers only"));
    accountName->setWhatsThis(tr("This name will be used to identify this user to the computer.  It must contain only letters and numbers."));
    nameLayout->addWidget(accountName, 15);

    QHBoxLayout *passAdminLayout = new QHBoxLayout;
    passphrase = new QLineEdit;
    passphrase->setEchoMode(QLineEdit::Password);
    passphrase->setPlaceholderText(tr("Passphrase"));
    passphrase->setToolTip(tr("This user's passphrase"));
    passphrase->setWhatsThis(tr("The passphrase will be used to log in to the computer."));
    QAction *togglePass = passphrase->addAction(QIcon::fromTheme("visibility"),
                                                QLineEdit::TrailingPosition);
    togglePass->setToolTip(tr("Show the passphrase"));
    togglePass->setData(true);
    connect(togglePass, &QAction::triggered,
            [=](void) {
        if(togglePass->data().toBool() == true) {
            togglePass->setData(false);
            togglePass->setIcon(QIcon::fromTheme("hint"));
            togglePass->setToolTip(tr("Hide the passphrase"));
            passphrase->setEchoMode(QLineEdit::Normal);
        } else {
            togglePass->setData(true);
            togglePass->setIcon(QIcon::fromTheme("visibility"));
            togglePass->setToolTip(tr("Show the passphrase"));
            passphrase->setEchoMode(QLineEdit::Password);
        }
    });
    passAdminLayout->addWidget(passphrase);

    adminTick = new QCheckBox(tr("Admin"));
    adminTick->setToolTip(tr("Allows this user to perform administrative tasks"));
    adminTick->setWhatsThis(tr("If ticked, allows this user to perform administrative tasks on the computer."));
    passAdminLayout->addWidget(adminTick);

    QVBoxLayout *detailLayout = new QVBoxLayout;
    detailLayout->addLayout(nameLayout);
    detailLayout->addLayout(passAdminLayout);

    QHBoxLayout *overallLayout = new QHBoxLayout;
    aviButton = new QPushButton;
    aviButton->setIcon(QIcon::fromTheme("user"));
    aviButton->setIconSize(QSize(32, 32));
    aviButton->setToolTip(tr("Change this user's avatar"));
    aviButton->setWhatsThis(tr("Allows you to choose the user's avatar, which will be shown on the log in screen."));
    overallLayout->addWidget(aviButton);
    overallLayout->addLayout(detailLayout);

    setLayout(overallLayout);

    connect(accountName, &QLineEdit::textEdited,
            [=]{
        emit validityChanged();
        this->acctEverTouched = true;
    });
    connect(personalName, &QLineEdit::textEdited,
            [=]{
        if(this->acctEverTouched) {
            emit validityChanged();
            return;
        }

        /* REQ: UI.Accounts.UserAcct.AcctName.Default */
        QString result = personalName->text()
                /* NFKC */
                .normalized(QString::NormalizationForm_KC)
                /* Casefold */
                .toLower();
        QStringList components = result.split(" ", QString::SkipEmptyParts);
        if(components.size() > 1) {
            result = "";
            for(int next = 0; next < components.size() - 1; next++) {
                result += components.at(next).left(1);
            }
            result += components.at(components.size() - 1);
        }
        /* if SkipEmptyParts causes components to be 1, but still has space */
        result.replace(" ", "");
        accountName->setText(result.left(32));
        emit validityChanged();
    });
    connect(passphrase, &QLineEdit::textEdited,
            [=]{ emit validityChanged(); });
}

QString UserAccountWidget::accountText(void) const {
    return accountName->text();
}

void UserAccountWidget::setAccountText(QString account) {
    accountName->setText(account);
    accountName->textEdited(account);
}

QString UserAccountWidget::passphraseText(void) const {
    return passphrase->text();
}

QString UserAccountWidget::personalText(void) const {
    return personalName->text();
}

void UserAccountWidget::setPersonalText(QString personal) {
    personalName->setText(personal);
    personalName->textEdited(personal);
}

bool UserAccountWidget::isAdmin(void) const {
    return adminTick->isChecked();
}

void UserAccountWidget::setAdmin(bool ticked) {
    adminTick->setChecked(ticked);
    /* adminTick being ticked can cause validity to change */
    emit validityChanged();
}

bool UserAccountWidget::isValid() const {
    /* The widget is valid if absolutely no account information has been specified. */
    if(accountText().isEmpty() && personalText().isEmpty() &&
            passphraseText().isEmpty() && !adminTick->isChecked()) {
        return true;
    }

    if(accountText().isEmpty() || personalText().isEmpty()) {
        return false;
    }

    return true;
}
