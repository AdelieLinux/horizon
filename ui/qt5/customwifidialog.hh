/*
 * customwifidialog.hh - Definition of a dialog for entering a custom AP
 * horizon-qt5, the Qt 5 user interface for
 * Project Horizon
 *
 * Copyright (c) 2019 Adélie Linux and contributors.  All rights reserved.
 * This code is licensed under the AGPL 3.0 license, as noted in the
 * LICENSE-code file in the root directory of this repository.
 *
 * SPDX-License-Identifier: AGPL-3.0-only
 */

#ifndef CUSTOMWIFIDIALOG_HH
#define CUSTOMWIFIDIALOG_HH

#include "horizonwizard.hh"

#include <QButtonGroup>
#include <QDialog>
#include <QLineEdit>
#include <QPushButton>
#include <QRadioButton>

class CustomWiFiDialog : public QDialog {
public:
    explicit CustomWiFiDialog(QWidget *parent = nullptr);

    void ensureButton();

    QString networkName() const;
    void setNetworkName(const QString &network);
    QStringList flags() const;
private:
    QPushButton *ok;
    QLineEdit *networkInput;
    QButtonGroup *securityGroup;
    QRadioButton *securityNone, *securityWEP, *securityWPA;
};

#endif  /* !CUSTOMWIFIDIALOG_HH */
