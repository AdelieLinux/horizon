/*
 * datetimepage.cc - Implementation of the UI.SysMeta.DateTime page
 * horizon-qt5, the Qt 5 user interface for
 * Project Horizon
 *
 * Copyright (c) 2019 Adélie Linux and contributors.  All rights reserved.
 * This code is licensed under the AGPL 3.0 license, as noted in the
 * LICENSE-code file in the root directory of this repository.
 *
 * SPDX-License-Identifier: AGPL-3.0-only
 */

#include "datetimepage.hh"

#include <QCheckBox>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QTimeZone>
#include <QVBoxLayout>

#ifdef HAS_INSTALL_ENV
#   include <sys/capability.h>
#   include <time.h>
#endif  /* HAS_INSTALL_ENV */


TimeZone::TimeZone() : ianaName(), friendlyName(), offset(0) {}

TimeZone::TimeZone(QByteArray iana) {
    QTimeZone zone(iana);
    QString offset, longName, comment;
    this->ianaName = iana;

    offset = zone.displayName(QTimeZone::GenericTime, QTimeZone::OffsetName);
    this->friendlyName = "(" + offset + ") " + iana;
    longName = zone.displayName(QTimeZone::GenericTime, QTimeZone::LongName);
    if(longName != offset) {
        this->friendlyName += " - " + longName;
    }

    comment = zone.comment();
    if(comment.size() > 0 && comment != offset) {
        this->friendlyName += " (" + comment + ")";
    }

    this->offset = zone.standardTimeOffset(QDateTime::currentDateTimeUtc());
}


TimeZoneModel::TimeZoneModel(QWidget *parent) : QAbstractListModel(parent) {
    for(auto &iana : QTimeZone::availableTimeZoneIds()) {
        /* we don't support raw timezones because tzdata doesn't */
        if(iana.startsWith("UTC") && iana.size() > 3) continue;
        TimeZone tzObj(iana);
        zones.push_back(tzObj);
    }

    std::sort(zones.begin(), zones.end(), [](TimeZone tz1, TimeZone tz2) {
        if(tz1.offset < tz2.offset) return true;
        if(tz2.offset < tz1.offset) return false;
        return tz1.friendlyName < tz2.friendlyName;
    });
}

int TimeZoneModel::rowCount(const QModelIndex &) const {
    return zones.size();
}

QVariant TimeZoneModel::data(const QModelIndex &index, int role) const {
    if(!index.isValid()) {
        return QVariant();
    }

    if(index.row() > zones.size()) {
        return QVariant();
    }

    TimeZone zone = zones.at(index.row());

    switch(role)
    {
    case Qt::DisplayRole:
        return zone.friendlyName;
    case Qt::ToolTipRole:
        return QString(zone.ianaName);
    default:
        return QVariant();
    }
}

QVariant TimeZoneModel::headerData(int, Qt::Orientation, int role) const {
    if(role != Qt::DisplayRole) {
        return QVariant();
    }

    return QString("Time Zone");
}


/*! Try to gain CAP_SYS_TIME capability.
 * If we do, then enable the date/time edit boxes.
 * This does nothing in the Runtime Environment.
 */
void DateTimePage::maybeRaiseCap() {
#ifdef HAS_INSTALL_ENV
    cap_t captain;
    cap_value_t time_cap = CAP_SYS_TIME;

    if(!CAP_IS_SUPPORTED(CAP_SYS_TIME))
        return;

    captain = cap_get_proc();
    if(captain == nullptr)
        return;

    if(cap_set_flag(captain, CAP_EFFECTIVE, 1, &time_cap, CAP_SET) == -1) {
        cap_free(captain);
        return;
    }

    if(cap_set_proc(captain)) {
        cap_free(captain);
        return;
    }

    cap_free(captain);

    dateEdit->setEnabled(true);
    timeEdit->setEnabled(true);
#endif  /* HAS_INSTALL_ENV */
}

DateTimePage::DateTimePage(QWidget *parent) : HorizonWizardPage(parent) {
    setTitle(tr("Date and Time Settings"));
    loadWatermark("intro");

    dateEdit = new QDateEdit(QDate::currentDate());
    dateEdit->setDisplayFormat("dd MMMM yyyy");
    dateEdit->setEnabled(false);
    timeEdit = new QTimeEdit(QTime::currentTime());
    timeEdit->setDisplayFormat("HH:mm:ss");
    timeEdit->setEnabled(false);

#ifdef HAS_INSTALL_ENV
    /* explanations:
     *
     * isEnabled check:
     *     to prevent wasting time in the case where we don't have perms
     *
     * Qt::UTC:
     *     CLOCK_REALTIME is always UTC, so we need the UTC form of the
     *     current date/time.
     */
    connect(dateEdit, &QDateEdit::dateChanged, [=](const QDate &date) {
        if(dateEdit->isEnabled() && date != QDate::currentDate()) {
            QDateTime *newDT = new QDateTime(date, QTime::currentTime(),
                                             Qt::UTC);
            struct timespec ts = {newDT->toSecsSinceEpoch(), 0};
            clock_settime(CLOCK_REALTIME, &ts);
        }
    });
    connect(timeEdit, &QTimeEdit::timeChanged, [=](const QTime &qtime) {
        if(timeEdit->isEnabled() && qtime != QTime::currentTime()) {
            QDateTime *newDT = new QDateTime(QDate::currentDate(), qtime);
            struct timespec ts = {newDT->toSecsSinceEpoch(), 0};
            clock_settime(CLOCK_REALTIME, &ts);
        }
    });
#endif  /* HAS_INSTALL_ENV */

    updateTimer = new QTimer(this);
    updateTimer->setInterval(1000);
    connect(updateTimer, &QTimer::timeout, [=]{
        dateEdit->setDate(QDate::currentDate());
        timeEdit->setTime(QTime::currentTime());
    });

    QHBoxLayout *dateTimeLayout = new QHBoxLayout;
    dateTimeLayout->addWidget(dateEdit);
    dateTimeLayout->addWidget(timeEdit);
    QGroupBox *dateTimeGroup = new QGroupBox(tr("Date and Time"));
    dateTimeGroup->setLayout(dateTimeLayout);

    QLineEdit *timeZoneSearch = new QLineEdit;
    timeZoneSearch->addAction(QIcon::fromTheme("edit-find"),
                              QLineEdit::LeadingPosition);
    timeZoneSearch->setPlaceholderText(tr("Search for a time zone"));
    sortModel = new QSortFilterProxyModel(this);
    sortModel->setFilterCaseSensitivity(Qt::CaseInsensitive);
    sortModel->setSourceModel(&zoneModel);
    connect(timeZoneSearch, &QLineEdit::textChanged, [=](const QString &text) {
        sortModel->setFilterFixedString(text);
    });
    timeZoneList = new QListView;
    timeZoneList->setModel(sortModel);
    timeZoneList->setSelectionMode(QAbstractItemView::SingleSelection);
    connect(timeZoneList->selectionModel(), &QItemSelectionModel::currentChanged,
            [=]() {
        emit timezoneChanged();
    });

    registerField("timezone*", this, "selectedTimeZone", SIGNAL(timezoneChanged()));

    QVBoxLayout *timeZoneLayout = new QVBoxLayout;
    timeZoneLayout->addWidget(timeZoneSearch);
    timeZoneLayout->addWidget(timeZoneList);
    QGroupBox *timeZoneGroup = new QGroupBox(tr("Time Zone"));
    timeZoneGroup->setLayout(timeZoneLayout);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addStretch();
    mainLayout->addWidget(dateTimeGroup);
    mainLayout->addStretch();
    mainLayout->addWidget(timeZoneGroup);
    mainLayout->addStretch();
    setLayout(mainLayout);

    maybeRaiseCap();
}

QString DateTimePage::selectedTimeZone() {
    QModelIndex curr = timeZoneList->selectionModel()->currentIndex();
    return sortModel->data(curr, Qt::ToolTipRole).toString();
}

void DateTimePage::initializePage() {
    updateTimer->start();
}

void DateTimePage::cleanupPage() {
    updateTimer->stop();
}
