/*
 * avatardialog.cc - Implementation of a dialog for selecting a user's avatar
 * horizon-qt5, the Qt 5 user interface for
 * Project Horizon
 *
 * Copyright (c) 2019 Adélie Linux and contributors.  All rights reserved.
 * This code is licensed under the AGPL 3.0 license, as noted in the
 * LICENSE-code file in the root directory of this repository.
 *
 * SPDX-License-Identifier: AGPL-3.0-only
 */

#include "avatardialog.hh"

#include <QDirIterator>
#include <QHBoxLayout>
#include <QListWidget>
#include <QPushButton>
#include <QVBoxLayout>

AvatarDialog::AvatarDialog(QWidget *parent)
    : QDialog(parent, Qt::WindowTitleHint | Qt::WindowCloseButtonHint) {
    setWindowTitle(tr("Choose Avatar"));

    aviList = new QListWidget;
    aviList->setGridSize(QSize(52, 52));
    aviList->setIconSize(QSize(48, 48));
    aviList->setViewMode(QListView::IconMode);
    loadAvatars();
    connect(aviList, &QListWidget::itemSelectionChanged, [=]{
        if(aviList->selectedItems().isEmpty()) return;
        path = aviList->selectedItems()[0]->data(Qt::ToolTipRole).toString();
    });

    QPushButton *ok = new QPushButton(tr("Choose"));
    connect(ok, &QPushButton::clicked, this, &QDialog::accept);
    QPushButton *cancel = new QPushButton(tr("Cancel"));
    connect(cancel, &QPushButton::clicked, this, &QDialog::reject);

    QVBoxLayout *buttonLayout = new QVBoxLayout;
    buttonLayout->setAlignment(Qt::AlignTop);
    buttonLayout->addWidget(ok);
    buttonLayout->addWidget(cancel);

    QHBoxLayout *mainBox = new QHBoxLayout;
    mainBox->addWidget(aviList);
    mainBox->addLayout(buttonLayout);

    setLayout(mainBox);
}

void AvatarDialog::loadAvatars() {
    QDirIterator it("/usr/share/user-manager/avatars", {}, QDir::Files, QDirIterator::Subdirectories);
    while(it.hasNext()) {
        QString path = it.next();
        QListWidgetItem *item = new QListWidgetItem(aviList);
        item->setData(Qt::ToolTipRole, path);
        item->setIcon(QPixmap(path));
    }
}

QString AvatarDialog::avatar() const {
    return this->path;
}

void AvatarDialog::setAvatar(QString path) {
    QList<QListWidgetItem *> items = aviList->findItems(path, Qt::MatchExactly);
    if(items.size() == 0)
        aviList->clearSelection();
    else
        items.at(0)->setSelected(true);
}
