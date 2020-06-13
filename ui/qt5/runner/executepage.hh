/*
 * executepage.hh - Defintion of the UI.Perform page
 * horizon-run-qt5, the Qt 5 executor user interface for
 * Project Horizon
 *
 * Copyright (c) 2019 Adélie Linux and contributors.  All rights reserved.
 * This code is licensed under the AGPL 3.0 license, as noted in the
 * LICENSE-code file in the root directory of this repository.
 *
 * SPDX-License-Identifier: AGPL-3.0-only
 */

#ifndef EXECUTEPAGE_HH
#define EXECUTEPAGE_HH

#include "../horizonwizardpage.hh"
#include "../stepprogresswidget.hh"

#include <QFile>
#include <QLabel>
#include <QProcess>
#include <QTimer>

class ExecutePage : public HorizonWizardPage {
public:
    enum Phase {
        Prepare,
        Validate,
        Disk,
        PreMeta,
        Net,
        Pkg,
        PostMeta
    };

    ExecutePage(QWidget *parent = nullptr);
    int nextId() const;
    Phase currentPhase() { return this->current; }

private:
    StepProgressWidget *progress;
    QProcess *executor;
    QTimer *finishTimer;
    QFile log;
    QByteArray msgBuffer;

    Phase current;
    bool failed;

    Phase stepToPhase(QString step);
    void markRunning(Phase phase);
    void markFinished(Phase phase);
    void markFailed(Phase phase);

    void processMessages();

    void executorReady();
    void executorOutReady();
    void executorErrored(QProcess::ProcessError what);
    void executorFinished(int code, QProcess::ExitStatus status);
};

#endif  /* !EXECUTEPAGE_HH */
