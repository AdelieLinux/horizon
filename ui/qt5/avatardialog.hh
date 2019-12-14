/*
 * avatardialog.hh - Definition of a dialog for selecting a user's avatar
 * horizon-qt5, the Qt 5 user interface for
 * Project Horizon
 *
 * Copyright (c) 2019 Adélie Linux and contributors.  All rights reserved.
 * This code is licensed under the AGPL 3.0 license, as noted in the
 * LICENSE-code file in the root directory of this repository.
 *
 * SPDX-License-Identifier: AGPL-3.0-only
 */

#ifndef AVATARDIALOG_HH
#define AVATARDIALOG_HH

#include <QDialog>
#include <QListWidget>

class AvatarDialog : public QDialog {
public:
    explicit AvatarDialog(QWidget *parent = nullptr);
    QString avatar(void) const;
    void setAvatar(QString path);
private:
    void loadAvatars();
    QListWidget *aviList;
    QString path;
};

#endif  /* !AVATARDIALOG_HH */
