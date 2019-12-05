/*
 * datetimepage.hh - Definition of the UI.SysMeta.DateTime page
 * horizon-qt5, the Qt 5 user interface for
 * Project Horizon
 *
 * Copyright (c) 2019 Adélie Linux and contributors.  All rights reserved.
 * This code is licensed under the AGPL 3.0 license, as noted in the
 * LICENSE-code file in the root directory of this repository.
 *
 * SPDX-License-Identifier: AGPL-3.0-only
 */

#ifndef DATETIMEPAGE_HH
#define DATETIMEPAGE_HH

#include "horizonwizardpage.hh"

#include <QAbstractListModel>
#include <QDateEdit>
#include <QListView>
#include <QSortFilterProxyModel>
#include <QTimeEdit>
#include <QTimer>
#include <QVector>

/*! Represents a single time zone */
struct TimeZone {
    /*! The IANA name, i.e. America/Chicago */
    QByteArray ianaName;
    /*! The 'friendly' name, i.e. 'Central Time' */
    QString friendlyName;
    /*! The UTC offset in seconds */
    int offset;

    TimeZone();
    TimeZone(QByteArray iana);
};

class TimeZoneModel : public QAbstractListModel {
public:
    TimeZoneModel(QWidget *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QVariant headerData(int section, Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const override;
private:
    QVector<TimeZone> zones;
};

class DateTimePage : public HorizonWizardPage {
    Q_OBJECT
    Q_PROPERTY(QString selectedTimeZone READ selectedTimeZone)
public:
    DateTimePage(QWidget *parent = nullptr);
    void initializePage();
    void cleanupPage();

    TimeZoneModel zoneModel;
    QSortFilterProxyModel *sortModel;
    QString selectedTimeZone();

signals:
    void timezoneChanged();

private:
    QDateEdit *dateEdit;
    QTimeEdit *timeEdit;
    QTimer *updateTimer;
    QListView *timeZoneList;

    void maybeRaiseCap();
};

#endif  /* !DATETIMEPAGE_HH */
