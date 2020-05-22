/*
 * mountdialog.cc - Implementation of a dialog for selecting a mount point
 * horizon-qt5, the Qt 5 user interface for
 * Project Horizon
 *
 * Copyright (c) 2020 Adélie Linux and contributors.  All rights reserved.
 * This code is licensed under the AGPL 3.0 license, as noted in the
 * LICENSE-code file in the root directory of this repository.
 *
 * SPDX-License-Identifier: AGPL-3.0-only
 */

#include "mountdialog.hh"

#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>

MountDialog::MountDialog(QStringList skipParts, QStringList skipMounts,
                         HorizonWizard *wizard, QWidget *parent)
    : QDialog(parent, Qt::WindowTitleHint | Qt::WindowCloseButtonHint) {
    setWindowTitle(tr("Choose Partition and Mount Point"));

    QPushButton *ok = new QPushButton(tr("Confirm"));
    ok->setEnabled(false);
    connect(ok, &QPushButton::clicked, this, &QDialog::accept);
    QPushButton *cancel = new QPushButton(tr("Cancel"));
    connect(cancel, &QPushButton::clicked, this, &QDialog::reject);

    QVBoxLayout *buttonLayout = new QVBoxLayout;
    buttonLayout->setAlignment(Qt::AlignTop);
    buttonLayout->addWidget(ok);
    buttonLayout->addWidget(cancel);

#ifdef HAS_INSTALL_ENV
    QStringList partitions;

    for(const auto &disk : wizard->disks) {
        if(disk.has_fs()) {
            partitions << QString::fromStdString(disk.node());
        } else if(disk.has_label()) {
            for(const auto &part : disk.partitions()) {
                partitions << QString::fromStdString(part.node());
            }
        }
    }

    QSet<QString> parts = partitions.toSet().subtract(skipParts.toSet());
    partitions = parts.toList();
    partitions.sort();

    partList = new QListWidget;
    partList->setWhatsThis(tr("A list of the partitions available on this system."));
    partList->addItems(partitions);
    connect(partList, &QListWidget::currentItemChanged, [=] {
        ok->setEnabled(partList->currentItem() != nullptr);
    });
#else
    partInput = new QLineEdit;
    partInput->setWhatsThis(tr("Input the name of a partition, such as /dev/sda1, here."));
#endif

    QStringList pathCandidates = {"/", "/home", "/opt", "/srv", "/usr",
                                  "/usr/local", "/var", "/var/db", "/var/log"};
    QSet<QString> paths = pathCandidates.toSet().subtract(skipMounts.toSet());
    pathCandidates = paths.toList();
    pathCandidates.sort();

    pathInput = new QComboBox;
    pathInput->setEditable(true);
    pathInput->setWhatsThis(tr("Select where the partition will be mounted."));
    pathInput->addItems(pathCandidates);

    QVBoxLayout *controlLayout = new QVBoxLayout;
    controlLayout->addWidget(new QLabel(tr("Partition")));
    controlLayout->addWidget(partList);
    controlLayout->addWidget(new QLabel(tr("will be mounted on")));
    controlLayout->addWidget(pathInput);

    QHBoxLayout *mainBox = new QHBoxLayout;
    mainBox->addLayout(controlLayout);
    mainBox->addLayout(buttonLayout);

    setLayout(mainBox);
}

QString MountDialog::partition() const {
    assert(partList->currentItem() != nullptr);
    return partList->currentItem()->text();
}

void MountDialog::setPartition(const QString &part) {
    QList<QListWidgetItem *> candidate = partList->findItems(part, Qt::MatchExactly);
    if(candidate.empty()) return;
    partList->setCurrentItem(candidate.at(0));
}

QString MountDialog::mountPoint() const {
    return pathInput->currentText();
}

void MountDialog::setMountPoint(const QString &path) {
    pathInput->setCurrentText(path);
}
