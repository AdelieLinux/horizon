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

#ifndef USERACCOUNTWIDGET_HH
#define USERACCOUNTWIDGET_HH

#include <QCheckBox>
#include <QLineEdit>
#include <QPushButton>
#include <QWidget>

class UserAccountWidget : public QWidget {
    Q_OBJECT
public:
    /*! Constructs a new User Account widget. */
    UserAccountWidget(QWidget *parent = nullptr);

    /*! Returns the current text in the Account Name text entry. */
    QString accountText(void) const;
    /*! Sets the current text in the Account Name text entry. */
    void setAccountText(QString);
    /*! Returns the current avatar location (or an empty string). */
    QString avatarPath(void) const;
    /*! Returns the current text in the passphrase entry. */
    QString passphraseText(void) const;
    /*! Returns the current text in the Personal Name text entry. */
    QString personalText(void) const;
    /*! Sets the current text in the Personal Name text entry. */
    void setPersonalText(QString);

    /*! Returns whether the Admin tickbox is ticked or not. */
    bool isAdmin(void) const;
    /*! Changes whether the Admin tickbox is ticked or not. */
    void setAdmin(bool);

    /*! Returns whether the widget defines a valid account. */
    bool isValid(void) const;
signals:
    void validityChanged();
private:
    QLineEdit *accountName;
    QLineEdit *personalName;
    QLineEdit *passphrase;
    QPushButton *aviButton;
    QCheckBox *adminTick;
    QString aviPath;
    /*! When set, auto-update of accountName from personalName is prevented. */
    bool acctEverTouched;
};

#endif  /* !USERACCOUNTWIDGET_HH */
