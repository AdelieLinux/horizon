/*
 * netdhcppage.cc - Implementation of the UI.Network.Automatic page
 * horizon-qt5, the Qt 5 user interface for
 * Project Horizon
 *
 * Copyright (c) 2019-2020 Adélie Linux and contributors.  All rights reserved.
 * This code is licensed under the AGPL 3.0 license, as noted in the
 * LICENSE-code file in the root directory of this repository.
 *
 * SPDX-License-Identifier: AGPL-3.0-only
 */

#include "netdhcppage.hh"
#include "horizonhelpwindow.hh"

#include <assert.h>
#include <QGridLayout>
#include <QGuiApplication>
#include <QUrl>
#include <QVBoxLayout>

NetDHCPPage::NetDHCPPage(QWidget *parent) : HorizonWizardPage(parent) {
    setTitle(tr("Automatic Network Configuration"));
    loadWatermark("network");

    information = new QLabel;
    information->setWordWrap(true);

    progress = new StepProgressWidget;
    progress->addStep(tr("Obtain a network address"));
    progress->addStep(tr("Check Internet connectivity"));

    logButton = new QPushButton(tr("Review DHCP Log"));
    logButton->setHidden(true);
    logButton->setWhatsThis(tr("Opens the DHCP client's log file in a log viewer."));
    connect(logButton, &QPushButton::clicked, [=]() {
        QFile logfile("/var/log/horizon/dhcpcd.log");
        logfile.open(QFile::ReadOnly);
        HorizonHelpWindow logview(&logfile, this, true);
        logview.exec();
    });

    authButton = new QPushButton(tr("Authenticate"));
    authButton->setHidden(true);
    authButton->setWhatsThis(tr("Opens a browser window where you can authenticate to this network."));
    connect(authButton, &QPushButton::clicked, [=] {
        QProcess p;
        p.execute("netsurf-gtk", {"http://distfiles.adelielinux.org/horizon.txt"});
    });

    QVBoxLayout *overallLayout = new QVBoxLayout(this);
    overallLayout->addWidget(progress);
    overallLayout->addSpacing(40);
    overallLayout->addWidget(information);
    overallLayout->addWidget(logButton, 0, Qt::AlignCenter);
    overallLayout->addWidget(authButton, 0, Qt::AlignCenter);
}

void NetDHCPPage::startDHCP() {
    QProcess *dhcpcd = new QProcess(this);
    QString iface(QString::fromStdString(horizonWizard()->chosen_auto_iface));
    dhcpcd->setProgram("/sbin/dhcpcd");
    dhcpcd->setArguments({"-q", "-t", "15", "-n",
                          "-j", "/var/log/horizon/dhcpcd.log", iface});
    connect(dhcpcd, &QProcess::errorOccurred,
            [=](QProcess::ProcessError error) {
        progress->setStepStatus(0, StepProgressWidget::Failed);
        if(error == QProcess::FailedToStart) {
            information->setText(tr("The Installation Environment is missing a critical component.  dhcpcd could not be loaded."));
            logButton->setHidden(true);
        } else {
            information->setText(tr("The system has encountered an internal issue."));
            logButton->setHidden(false);
        }
    });
    connect(dhcpcd, static_cast<void (QProcess:: *)(int, QProcess::ExitStatus)>(
                &QProcess::finished
            ), this, &NetDHCPPage::dhcpFinished);
    dhcpcd->start();
}

void NetDHCPPage::dhcpFinished(int exitcode, QProcess::ExitStatus status) {
    if(status != QProcess::NormalExit || exitcode != 0) {
        progress->setStepStatus(0, StepProgressWidget::Failed);
        information->setText(tr("The system could not obtain an address."));
        logButton->setHidden(false);
    } else {
        progress->stepPassed(0);
        checkInet();
    }
}

void NetDHCPPage::checkInet() {
    QString url = QString::fromStdString("http://" +
                                         horizonWizard()->mirror_domain +
                                         "/horizon.txt");
    inetReply = qnam.get(QNetworkRequest(QUrl(url)));
    connect(inetReply, &QNetworkReply::finished, this, &NetDHCPPage::inetFinished);
}

void NetDHCPPage::inetFinished() {
    QVariant redirUrl;
    QByteArray result;

    assert(inetReply);

    if(inetReply->error()) {
        progress->setStepStatus(1, StepProgressWidget::Failed);
        information->setText(tr("Couldn't connect to %1: %2")
                             .arg(QString::fromStdString(horizonWizard()->mirror_domain))
                             .arg(inetReply->errorString()));
        goto cleanReply;
    }

    redirUrl = inetReply->attribute(QNetworkRequest::RedirectionTargetAttribute);
    if(!redirUrl.isNull()) {
        progress->stepPassed(1);
        information->setText(tr("Received redirect to %2 while connecting to %1.  "
                                "You may need to authenticate to this network before continuing.  "
                                "Choose 'Authenticate' and close the browser window when you have successfully connected.")
                             .arg(QString::fromStdString(horizonWizard()->mirror_domain))
                             .arg(redirUrl.toUrl().toString()));
        authButton->setHidden(false);
        goto goOnline;
    }

    result = inetReply->readAll();
    if(result.size() != 3 || result[0] != 'O' || result[1] != 'K' ||
            result[2] != '\n') {
        QString res_str(result.left(512));
        if(result.size() > 512) res_str += "...";
        progress->setStepStatus(1, StepProgressWidget::Failed);
        information->setText(tr("Received unexpected %3 byte reply from %1: %2")
                             .arg(QString::fromStdString(horizonWizard()->mirror_domain))
                             .arg(res_str).arg(result.size()));
        goto goOnline;
    }

    progress->stepPassed(1);

    information->setText(tr("Your computer has successfully connected to the network.  You may now proceed."));

goOnline:
    online = true;
    emit completeChanged();

cleanReply:
    inetReply->deleteLater();
    inetReply = nullptr;
    return;
}

void NetDHCPPage::initializePage() {
    assert(!horizonWizard()->chosen_auto_iface.empty());

    progress->setStepStatus(0, StepProgressWidget::InProgress);

    if(QGuiApplication::queryKeyboardModifiers().testFlag(Qt::ControlModifier)) {
        online = true;
    }

    startDHCP();
}

bool NetDHCPPage::isComplete() const {
    return online;
}

int NetDHCPPage::nextId() const {
    return HorizonWizard::Page_DateTime;
}
