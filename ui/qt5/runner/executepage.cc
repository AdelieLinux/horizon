/*
 * executepage.cc - Implementation of the UI.Perform page
 * horizon-run-qt5, the Qt 5 executor user interface for
 * Project Horizon
 *
 * Copyright (c) 2019 Adélie Linux and contributors.  All rights reserved.
 * This code is licensed under the AGPL 3.0 license, as noted in the
 * LICENSE-code file in the root directory of this repository.
 *
 * SPDX-License-Identifier: AGPL-3.0-only
 */

#include "executepage.hh"

#include "executorwizard.hh"

#include <QAbstractButton>
#include <QGridLayout>
#include <QProcess>
#include <QTimer>
#include <QVBoxLayout>

ExecutePage::ExecutePage(QWidget *parent) : HorizonWizardPage(parent) {
    setTitle(tr("Installing Adélie Linux…"));
    loadWatermark("intro");
    failed = false;

    QLabel *descLabel = new QLabel(tr("Please wait while System Installation performs the following tasks:"));
    descLabel->setWordWrap(true);

    prepareStatus = new QLabel;
    prepare = new QLabel(tr("Prepare installation"));
    validateStatus = new QLabel;
    validate = new QLabel(tr("Validate installation"));
    diskStatus = new QLabel;
    disk = new QLabel(tr("Configure hard disk(s)"));
    preMetaStatus = new QLabel;
    preMeta = new QLabel(tr("Initial configuration"));
    netStatus = new QLabel;
    net = new QLabel(tr("Networking configuration"));
    pkgStatus = new QLabel;
    pkg = new QLabel(tr("Install software"));
    postMetaStatus = new QLabel;
    postMeta = new QLabel(tr("Final configuration"));

    QGridLayout *progressLayout = new QGridLayout;
    progressLayout->addWidget(prepareStatus, 0, 0);
    progressLayout->addWidget(prepare, 0, 1);
    progressLayout->addWidget(validateStatus, 1, 0);
    progressLayout->addWidget(validate, 1, 1);
    progressLayout->addWidget(diskStatus, 2, 0);
    progressLayout->addWidget(disk, 2, 1);
    progressLayout->addWidget(preMetaStatus, 3, 0);
    progressLayout->addWidget(preMeta, 3, 1);
    progressLayout->addWidget(netStatus, 4, 0);
    progressLayout->addWidget(net, 4, 1);
    progressLayout->addWidget(pkgStatus, 5, 0);
    progressLayout->addWidget(pkg, 5, 1);
    progressLayout->addWidget(postMetaStatus, 6, 0);
    progressLayout->addWidget(postMeta, 6, 1);
    progressLayout->setColumnStretch(1, 100);

    normalFont = validate->font();
    boldFont = normalFont;
    boldFont.setBold(true);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(descLabel);
    mainLayout->addStretch();
    mainLayout->addLayout(progressLayout);
    mainLayout->addStretch();

    setLayout(mainLayout);

    finishTimer = new QTimer(this);
    finishTimer->setInterval(1500);
    finishTimer->setSingleShot(true);
    connect(finishTimer, &QTimer::timeout, [=]{
        wizard()->next();
    });

    log.setFileName("/var/log/horizon/executor.log");
    Q_ASSERT(log.open(QFile::Append));

    this->current = Prepare;
    markRunning(this->current);

    executor = new QProcess(this);
    executor->setProgram("../../tools/hscript-simulate/hscript-simulate");
    executor->setArguments({"/etc/horizon/installfile"});
    connect(executor, &QProcess::readyReadStandardError,
            this, &ExecutePage::executorReady);
    connect(executor, &QProcess::readyReadStandardOutput,
            this, &ExecutePage::executorOutReady);
    connect(executor, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, &ExecutePage::executorFinished);
    executor->start();
}

ExecutePage::Phase ExecutePage::stepToPhase(QString step) {
    if(step == "validate") return Validate;
    if(step == "disk") return Disk;
    if(step == "pre-metadata") return PreMeta;
    if(step == "net") return Net;
    if(step == "pkgdb") return Pkg;
    if(step == "post-metadata") return PostMeta;
    Q_ASSERT(false);
    return Prepare;
}

void ExecutePage::labelsForPhase(Phase phase, QLabel **icon, QLabel **text) {
    switch(phase) {
    case Prepare:
        *icon = prepareStatus;
        *text = prepare;
        break;
    case Validate:
        *icon = validateStatus;
        *text = validate;
        break;
    case Disk:
        *icon = diskStatus;
        *text = disk;
        break;
    case PreMeta:
        *icon = preMetaStatus;
        *text = preMeta;
        break;
    case Net:
        *icon = netStatus;
        *text = net;
        break;
    case Pkg:
        *icon = pkgStatus;
        *text = pkg;
        break;
    case PostMeta:
        *icon = postMetaStatus;
        *text = postMeta;
        break;
    }
}

void ExecutePage::markRunning(Phase phase) {
    QLabel *icon, *text;
    labelsForPhase(phase, &icon, &text);
    icon->setPixmap(loadDPIAwarePixmap("status-current", ".svg"));
    text->setFont(boldFont);
}

void ExecutePage::markFinished(Phase phase) {
    QLabel *icon, *text;
    labelsForPhase(phase, &icon, &text);
    icon->setPixmap(loadDPIAwarePixmap("status-success", ".svg"));
    text->setFont(normalFont);
}

void ExecutePage::markFailed(Phase phase) {
    QLabel *icon, *text;
    labelsForPhase(phase, &icon, &text);
    icon->setPixmap(loadDPIAwarePixmap("status-issue", ".svg"));
    text->setFont(boldFont);
    failed = true;
}

void ExecutePage::executorReady() {
    QByteArray msgs = executor->readAllStandardError();
    log.write(msgs);

    QStringList msgList = QString::fromUtf8(msgs).split("\n");
    for(auto &msg : msgList) {
        QString msgType = msg.section('\t', 1, 1);
        if(msgType == "step-start") {
            this->current = stepToPhase(msg.section('\t', 2));
            /* validate means prepare is done */
            if(this->current == Validate) markFinished(Prepare);
            markRunning(this->current);
        } else if(msgType == "step-end") {
            Q_ASSERT(stepToPhase(msg.section('\t', 2)) == this->current);
            markFinished(this->current);
        } else if(msgType == "log") {
            QString severity = msg.section(": ", 1, 1);
            if(severity == "error") {
                markFailed(this->current);
            }
        } else {
            /* !? */
        }
    }
}

void ExecutePage::executorOutReady() {
    log.write(executor->readAllStandardOutput());
}

void ExecutePage::executorFinished(int code, QProcess::ExitStatus status) {
    if(status != QProcess::NormalExit || code != 0) {
        markFailed(this->current);
    }

    wizard()->button(QWizard::CancelButton)->setEnabled(false);
    finishTimer->start();
}

int ExecutePage::nextId() const {
    if(failed) {
        return ExecutorWizard::Page_Error;
    }
    return ExecutorWizard::Page_Finished;
}
