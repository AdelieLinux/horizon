/*
 * netdhcppage.cc - Implementation of the UI.Network.Automatic page
 * horizon-qt5, the Qt 5 user interface for
 * Project Horizon
 *
 * Copyright (c) 2019 Adélie Linux and contributors.  All rights reserved.
 * This code is licensed under the AGPL 3.0 license, as noted in the
 * LICENSE-code file in the root directory of this repository.
 *
 * SPDX-License-Identifier: AGPL-3.0-only
 */

#include "netdhcppage.hh"
#include "horizonhelpwindow.hh"

#include <assert.h>
#include <QGridLayout>
#include <QProcess>
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
    connect(logButton, &QPushButton::clicked, [=]() {
        QFile logfile("/var/log/horizon/dhcpcd.log");
        logfile.open(QFile::ReadOnly);
        HorizonHelpWindow logview(&logfile, this, true);
        logview.exec();
    });

    QVBoxLayout *overallLayout = new QVBoxLayout(this);
    overallLayout->addWidget(progress);
    overallLayout->addSpacing(40);
    overallLayout->addWidget(information);
    overallLayout->addWidget(logButton, 0, Qt::AlignCenter);
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
    connect(dhcpcd, static_cast<void (QProcess:: *)(int)>(&QProcess::finished),
            this, &NetDHCPPage::dhcpFinished);
    dhcpcd->start();
}

void NetDHCPPage::dhcpFinished(int exitcode) {
    if(exitcode != 0) {
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
    assert(inetReply);

    if(inetReply->error()) {
        progress->setStepStatus(1, StepProgressWidget::Failed);
        information->setText(tr("Couldn't connect to %1: %2")
                             .arg(QString::fromStdString(horizonWizard()->mirror_domain))
                             .arg(inetReply->errorString()));
        inetReply->deleteLater();
        inetReply = nullptr;
        return;
    }

    const QVariant redirUrl = inetReply->attribute(QNetworkRequest::RedirectionTargetAttribute);
    if(!redirUrl.isNull()) {
        progress->setStepStatus(1, StepProgressWidget::Failed);
        /* XXX TODO BAD UNIMPLEMENTED !!!! LOOK AT ME DO NOT RELEASE YET !!!! */
        information->setText(tr("Received redirect to %2 while connecting to %1."
                                "God help us if we don't ship Otter Browser and have to handle captive portals with QtWebKitWidgets.")
                             .arg(QString::fromStdString(horizonWizard()->mirror_domain))
                             .arg(redirUrl.toUrl().toString()));
        inetReply->deleteLater();
        inetReply = nullptr;
        return;
    }

    QByteArray result = inetReply->readAll();
    if(result.size() != 3 || result[0] != 'O' || result[1] != 'K' ||
            result[2] != '\n') {
        QString res_str(result.left(512));
        if(result.size() > 512) res_str += "...";
        progress->setStepStatus(1, StepProgressWidget::Failed);
        information->setText(tr("Received unexpected %3 byte reply from %1: %2")
                             .arg(QString::fromStdString(horizonWizard()->mirror_domain))
                             .arg(res_str).arg(result.size()));
        inetReply->deleteLater();
        inetReply = nullptr;
        return;
    }

    progress->stepPassed(1);

    information->setText(tr("Your computer has successfully connected to the network.  You may now proceed."));

    online = true;
    emit completeChanged();
}

void NetDHCPPage::initializePage() {
    assert(!horizonWizard()->chosen_auto_iface.empty());

    progress->setStepStatus(0, StepProgressWidget::InProgress);

    startDHCP();
}

bool NetDHCPPage::isComplete() const {
    return online;
}
