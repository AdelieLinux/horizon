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
#include <QSet>
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
    connect(partInput, &QLineEdit::textChanged, [=] {
        ok->setEnabled(partInput->text().startsWith("/dev/"));
    });
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

    QStringList fsCandidates = {"ext4", "xfs", "jfs", "hfs+", "vfat", "ext3"};
    formatChoice = new QCheckBox(tr("Format"));
    formatChoice->setWhatsThis(tr("Erase and format this device with a new file system."));

    formatInput = new QComboBox;
    formatInput->setEditable(false);
    formatInput->setWhatsThis(tr("Select the file system this partition will be formatted."));
    formatInput->addItems(fsCandidates);

    QHBoxLayout *formatLayout = new QHBoxLayout;
    formatLayout->addWidget(formatChoice);
    formatLayout->addStretch();
    formatLayout->addWidget(formatInput);

    QVBoxLayout *controlLayout = new QVBoxLayout;
    controlLayout->addWidget(new QLabel(tr("Partition")));
#ifdef HAS_INSTALL_ENV
    controlLayout->addWidget(partList);
#else  /* !HAS_INSTALL_ENV */
    controlLayout->addWidget(partInput);
#endif  /* HAS_INSTALL_ENV */
    controlLayout->addWidget(new QLabel(tr("will be mounted on")));
    controlLayout->addWidget(pathInput);
    controlLayout->addLayout(formatLayout);

    QHBoxLayout *mainBox = new QHBoxLayout;
    mainBox->addLayout(controlLayout);
    mainBox->addLayout(buttonLayout);

    setLayout(mainBox);
}

QString MountDialog::partition() const {
#ifdef HAS_INSTALL_ENV
    assert(partList->currentItem() != nullptr);
    return partList->currentItem()->text();
#else  /* !HAS_INSTALL_ENV */
    return partInput->text();
#endif  /* HAS_INSTALL_ENV */
}

void MountDialog::setPartition(const QString &part) {
#ifdef HAS_INSTALL_ENV
    QList<QListWidgetItem *> candidate = partList->findItems(part, Qt::MatchExactly);
    if(candidate.empty()) return;
    partList->setCurrentItem(candidate.at(0));
#else  /* !HAS_INSTALL_ENV */
    partInput->setText(part);
#endif  /* HAS_INSTALL_ENV */
}

QString MountDialog::mountPoint() const {
    return pathInput->currentText();
}

void MountDialog::setMountPoint(const QString &path) {
    pathInput->setCurrentText(path);
}

bool MountDialog::isFormatting() const {
    return formatChoice->isChecked();
}

void MountDialog::setFormatting(bool format) {
    formatChoice->setChecked(format);
}

QString MountDialog::formatType() const {
    return formatInput->currentText();
}

void MountDialog::setFormatType(const QString &formatType) {
    formatInput->setCurrentText(formatType);
}
