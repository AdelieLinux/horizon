/*
 * netdhcppage.hh - Definition of the UI.Network.Automatic page
 * horizon-qt5, the Qt 5 user interface for
 * Project Horizon
 *
 * Copyright (c) 2019-2020 Adélie Linux and contributors.  All rights reserved.
 * This code is licensed under the AGPL 3.0 license, as noted in the
 * LICENSE-code file in the root directory of this repository.
 *
 * SPDX-License-Identifier: AGPL-3.0-only
 */

#ifndef NETDHCPPAGE_HH
#define NETDHCPPAGE_HH

#include "horizonwizardpage.hh"
#include "stepprogresswidget.hh"

#include <QLabel>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QProcess>
#include <QPushButton>

class NetDHCPPage : public HorizonWizardPage {
public:
    explicit NetDHCPPage(QWidget *parent = nullptr);

    void initializePage() override;
    bool isComplete() const override;
    int nextId() const override;
private:
    StepProgressWidget *progress;
    QLabel *information;
    QPushButton *logButton, *authButton;

    QNetworkAccessManager qnam;
    QNetworkReply *inetReply;

    bool online = false;

    void startDHCP();
    void dhcpFinished(int exitcode, QProcess::ExitStatus status);
    void checkInet();
    void inetFinished();
};

#endif  /* !NETDHCPPAGE_HH */
