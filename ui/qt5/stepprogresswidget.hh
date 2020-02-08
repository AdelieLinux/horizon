/*
 * stepprogresswidget.hh -
 * Interface for a widget for displaying progress through a series of steps
 * horizon-qt5, the Qt 5 user interface for
 * Project Horizon
 *
 * Copyright (c) 2020 Adélie Linux and contributors.  All rights reserved.
 * This code is licensed under the AGPL 3.0 license, as noted in the
 * LICENSE-code file in the root directory of this repository.
 *
 * SPDX-License-Identifier: AGPL-3.0-only
 */

#ifndef STEPPROGRESSWIDGET_HH
#define STEPPROGRESSWIDGET_HH

#include <QGridLayout>
#include <QLabel>
#include <QWidget>

class StepProgressWidget : public QWidget {
public:
    enum Status {
        NotStarted,
        InProgress,
        Finished,
        Failed
    };

    StepProgressWidget(QWidget *parent = nullptr);

    /*! Returns the current overview text. */
    QString overviewText();
    /*! Sets a custom overview text for the operations being performed. */
    void setOverviewText(QString overview);

    /*! Add a step to this progress widget. */
    int16_t addStep(QString stepInfo);
    /*! Set the status of a step. */
    void setStepStatus(int16_t step, Status status);
    /*! Sets the status of +step+ as Finished, and the following step
     *  InProgress.  This is a short hand for:
     *
     *  setStepStatus(step, Finished);
     *  setStepStatus(step + 1, InProgress);
     */
    void stepPassed(int16_t step);
private:
    /*! The overview text widget */
    QLabel *overview;

    /*! Grid layout with the steps */
    QGridLayout *stepGrid;

    /*! All the step status icons */
    QVector<QLabel *> statuses;
    /*! All the step info labels */
    QVector<QLabel *> infos;

    /*! The fonts used for infos */
    QFont normalFont, boldFont;

    QPixmap loadDPIAwarePixmap(QString pixmap, QString type);
};

#endif /* !STEPPROGRESSWIDGET_HH */
