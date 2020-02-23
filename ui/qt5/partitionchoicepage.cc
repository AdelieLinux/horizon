/*
 * partitionchoicepage.cc - Implementation of UI.Partition.Install.UserPrompt
 * horizon-qt5, the Qt 5 user interface for
 * Project Horizon
 *
 * Copyright (c) 2020 Adélie Linux and contributors.  All rights reserved.
 * This code is licensed under the AGPL 3.0 license, as noted in the
 * LICENSE-code file in the root directory of this repository.
 *
 * SPDX-License-Identifier: AGPL-3.0-only
 */

#include "partitionchoicepage.hh"

#include <QLabel>
#include <QMessageBox>
#include <QVBoxLayout>

PartitionChoicePage::PartitionChoicePage(QWidget *parent)
    : HorizonWizardPage(parent) {
    loadWatermark("disk");
    setTitle(tr("Choose Partitioning Type"));

    descLabel = new QLabel(tr("Select the method you wish to use for partitioning %1:"));
    descLabel->setWordWrap(true);

    eraseButton = new QRadioButton(tr("&Erase and Use Whole Disk"));
    eraseButton->setHidden(true);
    connect(eraseButton, &QRadioButton::clicked, [=] {
        horizonWizard()->auto_part = true;
        horizonWizard()->erase = true;
    });
    eraseLabel = new QLabel(tr("The entire disk will be erased and then automatically partitioned.<br><strong>Warning: This will destroy all existing data on the disk.</strong>"));
    eraseLabel->setHidden(true);
    eraseLabel->setIndent(25);
    eraseLabel->setTextFormat(Qt::RichText);
    eraseLabel->setWordWrap(true);

    fitInButton = new QRadioButton(tr("Use &Free Space"));
    fitInButton->setHidden(true);
    connect(fitInButton, &QRadioButton::clicked, [=] {
        horizonWizard()->auto_part = true;
        horizonWizard()->erase = false;
    });
    fitInLabel = new QLabel(tr("The free space on the disk will be automatically partitioned for use with Adélie.  Existing data will be preserved."));
    fitInLabel->setHidden(true);
    fitInLabel->setIndent(25);
    fitInLabel->setWordWrap(true);

    useExistingButton = new QRadioButton(tr("Use Existing &Partition"));
    useExistingButton->setHidden(true);
    connect(useExistingButton, &QRadioButton::clicked, [=] {
        horizonWizard()->auto_part = false;
        horizonWizard()->erase = false;
    });
    useExistingLabel = new QLabel(tr("No partitions will be modified.  You must select the partition on which you wish to install Adélie."));
    useExistingLabel->setHidden(true);
    useExistingLabel->setIndent(25);
    useExistingLabel->setWordWrap(true);

    manualButton = new QRadioButton(tr("&Manual"));
    manualButton->setHidden(true);
    connect(manualButton, &QRadioButton::clicked, [=] {
        horizonWizard()->auto_part = false;
        horizonWizard()->erase = false;
    });
    manualLabel = new QLabel(tr("Open a partitioning tool."));
    manualLabel->setHidden(true);
    manualLabel->setIndent(25);
    manualLabel->setWordWrap(true);

    buttons = new QButtonGroup(this);
    buttons->setExclusive(true);
    buttons->addButton(eraseButton);
    buttons->addButton(fitInButton);
    buttons->addButton(useExistingButton);
    buttons->addButton(manualButton);

    connect(buttons, QOverload<QAbstractButton *>::of(&QButtonGroup::buttonClicked),
            this, &PartitionChoicePage::completeChanged);

    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(descLabel);
    layout->addStretch();
    layout->addWidget(eraseButton);
    layout->addWidget(eraseLabel);
    layout->addStretch();
    layout->addWidget(fitInButton);
    layout->addWidget(fitInLabel);
    layout->addStretch();
    layout->addWidget(useExistingButton);
    layout->addWidget(useExistingLabel);
    layout->addStretch();
    layout->addWidget(manualButton);
    layout->addWidget(manualLabel);
    layout->addStretch();
    setLayout(layout);
}

void PartitionChoicePage::initializePage() {
    Q_ASSERT(horizonWizard()->chosen_disk.size() > 0);

    /* these options are, as of right now, always available */
    eraseButton->setHidden(false);
    eraseLabel->setHidden(false);
    manualButton->setHidden(false);
    manualLabel->setHidden(false);

#ifdef HAS_INSTALL_ENV
    Horizon::DiskMan::Disk *d = nullptr;
    for(auto &disk : horizonWizard()->disks) {
        if(disk.node() == horizonWizard()->chosen_disk) {
            d = &disk;
            break;
        }
    }

    Q_ASSERT(d != nullptr);
    if(d->has_label()) {
        useExistingButton->setHidden(false);
        useExistingLabel->setHidden(false);

        if(d->contiguous_block() > 2000) {
            fitInButton->setHidden(false);
            fitInLabel->setHidden(false);
        }
    } else {
        if(d->has_fs()) {
            useExistingButton->setHidden(false);
            useExistingLabel->setHidden(false);
        } else {
            /* No label and no FS. */
            descLabel->setText(tr("The disk at %1 does not contain a recognised disklabel or file system.  "
                                  "It may be empty, or it may contain data that isn't readable by System Installation.\n\n"
                                  "Continuing will erase any data present on the disk, if any."));
        }
    }
#else  /* !HAS_INSTALL_ENV */
    /* Go ahead and give the user the choice to use existing. */
    useExistingButton->setHidden(false);
    useExistingLabel->setHidden(false);
#endif  /* HAS_INSTALL_ENV */

    QString chosen{QString::fromStdString(horizonWizard()->chosen_disk)};
    descLabel->setText(descLabel->text().arg(chosen));
}

bool PartitionChoicePage::isComplete() const {
    return buttons->checkedButton() != nullptr;
}

int PartitionChoicePage::nextId() const {
    if(buttons->checkedButton() == manualButton) {
        return HorizonWizard::Page_PartitionManual;
    } else if(buttons->checkedButton() == useExistingButton) {
        return HorizonWizard::Page_PartitionMount;
    } else {
        return HorizonWizard::Page_Network;
    }
}

bool PartitionChoicePage::validatePage() {
    if(buttons->checkedButton() == eraseButton) {
        return (QMessageBox::critical(this, tr("Erase Disk"),
                tr("You have chosen to ERASE %1.  "
                   "All data on %1 will be permanently erased.\n\n"
                   "Do you wish to continue?").arg(QString::fromStdString(horizonWizard()->chosen_disk)),
                QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes);
    }

    return true;
}
