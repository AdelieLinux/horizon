/*
 * horizonhelpwindow.hh - Definition of the Help window
 * horizon-qt5, the Qt 5 user interface for
 * Project Horizon
 *
 * Copyright (c) 2019 Adélie Linux and contributors.  All rights reserved.
 * This code is licensed under the AGPL 3.0 license, as noted in the
 * LICENSE-code file in the root directory of this repository.
 *
 * SPDX-License-Identifier: AGPL-3.0-only
 */

#ifndef HORIZONHELPWINDOW_HH
#define HORIZONHELPWINDOW_HH

#include <QDialog>
#include <QFile>

class HorizonHelpWindow : public QDialog {
    Q_OBJECT
public:
    explicit HorizonHelpWindow(QFile *helpFile, QWidget *parent = nullptr, bool log = false);
private:
    QFile *helpFile;
};

#endif  /* !HORIZONHELPWINDOW_HH */
