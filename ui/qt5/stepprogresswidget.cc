/*
 * stepprogresswidget.cc -
 * Implementation of a widget for displaying progress through a series of steps
 * horizon-qt5, the Qt 5 user interface for
 * Project Horizon
 *
 * Copyright (c) 2020 Adélie Linux and contributors.  All rights reserved.
 * This code is licensed under the AGPL 3.0 license, as noted in the
 * LICENSE-code file in the root directory of this repository.
 *
 * SPDX-License-Identifier: AGPL-3.0-only
 */

#include "stepprogresswidget.hh"

#include "horizonwizardpage.hh"

#include <QVBoxLayout>

StepProgressWidget::StepProgressWidget(QWidget *parent) : QWidget(parent) {
    overview = new QLabel(tr("Please wait while System Installation "
                             "performs the following tasks:"));
    overview->setWordWrap(true);

    /* Initialise the normal and bold fonts using the overview label. */
    QFont myFont = overview->font();
    myFont.setBold(false);
    normalFont = myFont;
    myFont.setBold(true);
    boldFont = myFont;

    setWhatsThis(tr("Displays the current status of System Installation tasks."));

    stepGrid = new QGridLayout;
    stepGrid->setColumnStretch(1, 100);

    QVBoxLayout *overallLayout = new QVBoxLayout;
    overallLayout->addWidget(overview);
    overallLayout->addSpacing(40);
    overallLayout->addLayout(stepGrid);
    setLayout(overallLayout);
}

QString StepProgressWidget::overviewText() {
    return overview->text();
}

void StepProgressWidget::setOverviewText(QString text) {
    overview->setText(text);
}

int16_t StepProgressWidget::addStep(QString stepInfo) {
    QLabel *status = new QLabel;
    QLabel *info = new QLabel(stepInfo);

    statuses.push_back(status);
    infos.push_back(info);

    int16_t row = infos.size();

    stepGrid->addWidget(status, row, 0);
    stepGrid->addWidget(info, row, 1);

    return row;
}

QPixmap StepProgressWidget::loadDPIAwarePixmap(QString pixmap, QString type) {
    QString path = ":/wizard_pixmaps/resources/";
    path += pixmap;
    path += "-";

    if(this->devicePixelRatioF() <= 1.0) {
        path += "low";
    } else {
        path += "high";
    }

    path += type;
    return QPixmap(path);
}

void StepProgressWidget::setStepStatus(int16_t step, Status status) {
    QLabel *statLabel = statuses.at(step);
    QLabel *infoLabel = infos.at(step);

    switch(status) {
    case NotStarted:
        statLabel->clear();
        infoLabel->setFont(normalFont);
        break;
    case InProgress:
        statLabel->setPixmap(loadDPIAwarePixmap("status-current", ".svg"));
        infoLabel->setFont(boldFont);
        break;
    case Finished:
        statLabel->setPixmap(loadDPIAwarePixmap("status-success", ".svg"));
        infoLabel->setFont(normalFont);
        break;
    case Failed:
        statLabel->setPixmap(loadDPIAwarePixmap("status-issue", ".svg"));
        infoLabel->setFont(boldFont);
        break;
    }
}

void StepProgressWidget::stepPassed(int16_t step) {
    setStepStatus(step, Finished);

    if(step + 1 < statuses.size()) {
        setStepStatus(step + 1, InProgress);
    }
}
