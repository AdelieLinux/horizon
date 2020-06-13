/*
 * executepage.cc - Implementation of the UI.Perform page
 * horizon-run-qt5, the Qt 5 executor user interface for
 * Project Horizon
 *
 * Copyright (c) 2019-2020 Adélie Linux and contributors.  All rights reserved.
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

    progress = new StepProgressWidget;
    progress->addStep(tr("Prepare installation"));
    progress->addStep(tr("Validate installation"));
    progress->addStep(tr("Configure hard disk(s)"));
    progress->addStep(tr("Initial configuration"));
    progress->addStep(tr("Networking configuration"));
    progress->addStep(tr("Install software"));
    progress->addStep(tr("Final configuration"));

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(progress);
    mainLayout->addStretch();

    setLayout(mainLayout);

    finishTimer = new QTimer(this);
    finishTimer->setInterval(5000);
    finishTimer->setSingleShot(true);
    connect(finishTimer, &QTimer::timeout, [=]{
        wizard()->next();
    });

    log.setFileName("/var/log/horizon/executor.log");
    log.open(QFile::Append);

    this->current = Prepare;
    markRunning(this->current);

    executor = new QProcess(this);
    executor->setProgram("hscript-executor");
    connect(executor, &QProcess::readyReadStandardError,
            this, &ExecutePage::executorReady);
    connect(executor, &QProcess::readyReadStandardOutput,
            this, &ExecutePage::executorOutReady);
    connect(executor, &QProcess::errorOccurred,
            this, &ExecutePage::executorErrored);
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

void ExecutePage::markRunning(Phase phase) {
    progress->setStepStatus(phase, StepProgressWidget::InProgress);
}

void ExecutePage::markFinished(Phase phase) {
    progress->setStepStatus(phase, StepProgressWidget::Finished);
}

void ExecutePage::markFailed(Phase phase) {
    progress->setStepStatus(phase, StepProgressWidget::Failed);
    failed = true;
}

void ExecutePage::executorReady() {
    QByteArray msgs = executor->readAllStandardError();
    log.write(msgs);

    msgBuffer.append(msgs);
    processMessages();
}

void ExecutePage::processMessages() {
    QByteArray ready;

    /* No message is ready yet. */
    if(!msgBuffer.contains('\n')) return;

    int len = msgBuffer.lastIndexOf('\n');
    ready = msgBuffer.left(len);
    msgBuffer.remove(0, len + 1);

    QStringList msgList = QString::fromUtf8(ready).split("\n");
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

    log.flush();
    wizard()->button(QWizard::CancelButton)->setEnabled(false);
    finishTimer->start();
}

void ExecutePage::executorErrored(QProcess::ProcessError what) {
    markFailed(this->current);
    wizard()->button(QWizard::CancelButton)->setEnabled(false);
    finishTimer->start();
}

int ExecutePage::nextId() const {
    if(failed) {
        return ExecutorWizard::Page_Error;
    }
    return ExecutorWizard::Page_Finished;
}
