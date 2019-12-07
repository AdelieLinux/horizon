/*
 * executorwizard.cc - Implementation of the wizard class
 * horizon-run-qt5, the Qt 5 executor user interface for
 * Project Horizon
 *
 * Copyright (c) 2019 Adélie Linux and contributors.  All rights reserved.
 * This code is licensed under the AGPL 3.0 license, as noted in the
 * LICENSE-code file in the root directory of this repository.
 *
 * SPDX-License-Identifier: AGPL-3.0-only
 */

#ifndef EXECUTORWIZARD_HH
#define EXECUTORWIZARD_HH

#include <QWizard>

class ExecutorWizard : public QWizard {
public:
    enum {
        Page_Execute,
        Page_Finished
    };

    ExecutorWizard(QWidget *parent = nullptr);
    void reject();
};

#endif  /* !EXECUTORWIZARD_HH */
