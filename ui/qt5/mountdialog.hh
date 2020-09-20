/*
 * mountdialog.hh - Definition of a dialog for selecting a mount point
 * horizon-qt5, the Qt 5 user interface for
 * Project Horizon
 *
 * Copyright (c) 2019 Adélie Linux and contributors.  All rights reserved.
 * This code is licensed under the AGPL 3.0 license, as noted in the
 * LICENSE-code file in the root directory of this repository.
 *
 * SPDX-License-Identifier: AGPL-3.0-only
 */

#ifndef MOUNTDIALOG_HH
#define MOUNTDIALOG_HH

#include "horizonwizard.hh"

#include <QCheckBox>
#include <QComboBox>
#include <QDialog>
#ifdef HAS_INSTALL_ENV
#include <QListWidget>
#else  /* !HAS_INSTALL_ENV */
#include <QLineEdit>
#endif  /* HAS_INSTALL_ENV */

class MountDialog : public QDialog {
public:
    explicit MountDialog(QStringList skipParts, QStringList skipMounts,
                         HorizonWizard *wizard, QWidget *parent = nullptr);

    QString partition() const;
    void setPartition(const QString &part);
    QString mountPoint() const;
    void setMountPoint(const QString &path);

    bool isFormatting() const;
    void setFormatting(bool format);
    QString formatType() const;
    void setFormatType(const QString &formatType);
private:
#ifdef HAS_INSTALL_ENV
    QListWidget *partList;
#else  /* !HAS_INSTALL_ENV */
    QLineEdit *partInput;
#endif  /* HAS_INSTALL_ENV */
    QComboBox *pathInput;
    QCheckBox *formatChoice;
    QComboBox *formatInput;
    QString path;
};

#endif  /* !MOUNTDIALOG_HH */
