/*
 * partitionmountpage.cc - Implementation of UI.Partition.Install.Mount page
 * horizon-qt5, the Qt 5 user interface for
 * Project Horizon
 *
 * Copyright (c) 2020 Adélie Linux and contributors.  All rights reserved.
 * This code is licensed under the AGPL 3.0 license, as noted in the
 * LICENSE-code file in the root directory of this repository.
 *
 * SPDX-License-Identifier: AGPL-3.0-only
 */

#include "partitionmountpage.hh"
#include "mountdialog.hh"

#include <functional>
#include <QHBoxLayout>
#include <QVBoxLayout>

PartitionMountPage::PartitionMountPage(QWidget *parent)
    : HorizonWizardPage(parent) {
    loadWatermark("disk");
    setTitle(tr("Set Mount Points"));

    mountList = new QListWidget;

    QVBoxLayout *buttonLayout = new QVBoxLayout;
    addMountButton = new QPushButton(tr("&Add Mount..."));
    addMountButton->setIcon(QIcon::fromTheme("list-add"));
    connect(addMountButton, &QPushButton::clicked, [=] {
        QStringList parts, paths;
        for(const auto &mount : mountList->findItems("", Qt::MatchContains)) {
            parts << mount->data(Qt::UserRole + 1).toString();
            paths << mount->data(Qt::UserRole + 2).toString();
        }
        MountDialog md(parts, paths, horizonWizard());
        if(md.exec() == QDialog::Accepted) {
            QListWidgetItem *mount = new QListWidgetItem;
            QString part = md.partition();
            QString path = md.mountPoint();
            QString fs = md.formatType();
            mount->setText(tr("%1 on %2").arg(part).arg(path));
            mount->setIcon(QIcon::fromTheme("drive-harddisk"));
            mount->setData(Qt::UserRole + 1, part);
            mount->setData(Qt::UserRole + 2, path);
            mount->setData(Qt::UserRole + 3, md.isFormatting());
            mount->setData(Qt::UserRole + 4, fs);

            mountList->addItem(mount);
        }
    });
    buttonLayout->addWidget(addMountButton);

    delMountButton = new QPushButton(tr("&Remove Mount"));
    delMountButton->setEnabled(false);
    delMountButton->setIcon(QIcon::fromTheme("list-remove"));
    connect(delMountButton, &QPushButton::clicked, [=] {
        delete mountList->takeItem(mountList->currentRow());
    });
    buttonLayout->addWidget(delMountButton);

    /*rescanButton = new QPushButton(tr("Re&scan Devices"));
    rescanButton->setIcon(QIcon::fromTheme("view-refresh"));
    connect(rescanButton, &QPushButton::clicked, [=] {

    });
    buttonLayout->addWidget(rescanButton);*/

    connect(mountList, &QListWidget::currentItemChanged, [=] {
        delMountButton->setEnabled(mountList->currentItem() != nullptr);
    });

    std::function<void()> listRowsChanged { [=] {
        emit completeChanged();
    } };
    connect(mountList->model(), &QAbstractItemModel::rowsInserted, listRowsChanged);
    connect(mountList->model(), &QAbstractItemModel::rowsRemoved, listRowsChanged);

    QHBoxLayout *layout = new QHBoxLayout;
    layout->addStretch();
    layout->addWidget(mountList);
    layout->addLayout(buttonLayout);
    layout->addStretch();

    setLayout(layout);
}

bool PartitionMountPage::isComplete() const {
    return !mountList->findItems(" on /", Qt::MatchEndsWith).isEmpty();
}

QStringList PartitionMountPage::mountLines() const {
    QStringList lines;
    for(const auto &mount : mountList->findItems("", Qt::MatchContains)) {
        QString part = mount->data(Qt::UserRole + 1).toString();
        QString path = mount->data(Qt::UserRole + 2).toString();
        QString fs = mount->data(Qt::UserRole + 4).toString();

        if(mount->data(Qt::UserRole + 3).toBool()) {
            lines << QString("fs %1 %2").arg(part).arg(fs);
        }

        lines << QString("mount %1 %2").arg(part).arg(path);
    }
    return lines;
}
