/*
 * partitionmanualpage.cc - Implementation of UI.Partition.Install.Manual page
 * horizon-qt5, the Qt 5 user interface for
 * Project Horizon
 *
 * Copyright (c) 2020 Adélie Linux and contributors.  All rights reserved.
 * This code is licensed under the AGPL 3.0 license, as noted in the
 * LICENSE-code file in the root directory of this repository.
 *
 * SPDX-License-Identifier: AGPL-3.0-only
 */

#include "partitionmanualpage.hh"

#include <QLabel>
#ifdef HAS_INSTALL_ENV
#   include <QProcess>
#   include <QPushButton>
#endif  /* HAS_INSTALL_ENV */
#include <QVBoxLayout>

PartitionManualPage::PartitionManualPage(QWidget *parent)
    : HorizonWizardPage(parent) {
    QLabel *descLabel = new QLabel;
    descLabel->setWordWrap(true);

    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(descLabel);

    loadWatermark("disk");
#ifdef HAS_INSTALL_ENV
    setTitle(tr("Launch Manual Partitioner"));

    descLabel->setText(tr("Choose 'Launch Partitioner' to launch the manual partitioning utility.\n\n"
                          "When you have finished partitioning the disk, quit the partitioning utility and choose Next."));

    QPushButton *button = new QPushButton(tr("Launch Partitioner"));
    connect(button, &QPushButton::clicked, [=]{
        QProcess p;
        p.execute("partitionmanager");
    });
    layout->addStretch();
    layout->addWidget(button, 0, Qt::AlignCenter);
    layout->addStretch();
#else  /* !HAS_INSTALL_ENV */
    setTitle(tr("Enter Partitioning Information"));

    descLabel->setText(tr("Enter the partitioning commands you wish to use for the target computer.\n\n"
                          "For a list of valid commands, choose Help or review the HorizonScript Reference."));

    partitionEdit = new QTextEdit;
    connect(partitionEdit, &QTextEdit::textChanged, [=]{
        horizonWizard()->part_lines = partitionEdit->toPlainText().split("\n");
        emit completeChanged();
    });
    partitionEdit->setAcceptRichText(false);
    partitionEdit->setFontFamily("monospace");
    partitionEdit->setReadOnly(false);
    layout->addWidget(partitionEdit);
#endif  /* HAS_INSTALL_ENV */

    setLayout(layout);
}

#ifndef HAS_INSTALL_ENV
bool PartitionManualPage::isComplete() const {
    return partitionEdit->toPlainText().size() > 0;
}
#endif  /* !HAS_INSTALL_ENV */
