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

#include "executorwizard.hh"

#include <QAbstractButton>

#include "executepage.hh"
#include "errorpage.hh"
#include "finishedpage.hh"

ExecutorWizard::ExecutorWizard(QWidget *parent, bool _autom)
    : QWizard(parent), automatic(_autom) {
    setWindowTitle(tr("Adélie Linux System Installation"));

    setFixedSize(QSize(650, 450));

    setPage(Page_Execute, new ExecutePage);
    setPage(Page_Error, new ErrorPage);
    setPage(Page_Finished, new FinishedPage);

    setOption(NoBackButtonOnStartPage);
    setOption(NoBackButtonOnLastPage);
    setOption(NoCancelButtonOnLastPage);

    QList<QWizard::WizardButton> buttonLayout;
    buttonLayout << Stretch << FinishButton;
    setButtonLayout(buttonLayout);
}

/* just plain don't allow rejection (Esc/F3/Cancel) */
void ExecutorWizard::reject() {
    return;
}
