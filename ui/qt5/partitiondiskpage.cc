/*
 * partitiondiskpage.cc - Implementation of UI.Partition.Install.UserPrompt
 * horizon-qt5, the Qt 5 user interface for
 * Project Horizon
 *
 * Copyright (c) 2020 Adélie Linux and contributors.  All rights reserved.
 * This code is licensed under the AGPL 3.0 license, as noted in the
 * LICENSE-code file in the root directory of this repository.
 *
 * SPDX-License-Identifier: AGPL-3.0-only
 */

#include "partitiondiskpage.hh"

#include <QLabel>
#include <QVBoxLayout>
#include "partitionchoicepage.hh"

QIcon iconForDisk(Horizon::DiskMan::Disk disk) {
    QString iconName;
    if(disk.dev_path().find("usb") != std::string::npos) {
        iconName = "drive-removable-media-usb";
    } else if(disk.dev_path().find("firewire") != std::string::npos) {
        iconName = "drive-harddisk-ieee1394";
    } else if(disk.node().find("mmcblk") != std::string::npos) {
        iconName = "media-flash-sd-mmc";
    } else if(disk.node().find("/md") != std::string::npos) {
        iconName = "drive-multidisk";
    } else if(disk.node().find("nvme") != std::string::npos) {
        /* this is papirus-specific */
        iconName = "gnome-dev-memory";
    } else {
        iconName = "drive-harddisk";
    }

    return QIcon::fromTheme(iconName);
}

PartitionDiskPage::PartitionDiskPage(QWidget *parent)
    : HorizonWizardPage(parent) {
    loadWatermark("disk");
    setTitle(tr("Select Installation Disk"));

    QLabel *descLabel = new QLabel(tr("Choose the hard disk on which to install Adélie Linux."));
    descLabel->setWordWrap(true);

    diskChooser = new QListWidget;
    connect(diskChooser, &QListWidget::currentItemChanged, [=]{
        if(diskChooser->currentItem() != nullptr) {
            QVariant itemData = diskChooser->currentItem()->data(Qt::UserRole);
            horizonWizard()->chosen_disk = itemData.toString().toStdString();

            /* ensure that the Choice page receives our new disk information */
            horizonWizard()->removePage(HorizonWizard::Page_PartitionChoose);
            horizonWizard()->setPage(HorizonWizard::Page_PartitionChoose,
                                     new PartitionChoicePage);
        }
        emit completeChanged();
    });
    diskChooser->setAutoFillBackground(true);
    diskChooser->setFrameShape(QFrame::NoFrame);
    diskChooser->setIconSize(QSize(32, 32));
    diskChooser->setMovement(QListView::Static);
    QColor color = this->palette().background().color();
    diskChooser->setStyleSheet(QString{"QListView { background: rgb(%1, %2, %3); }"}
                               .arg(color.red()).arg(color.green()).arg(color.blue()));
    diskChooser->setWrapping(true);
    diskChooser->setWhatsThis(tr("This is a list of hard disk drives found in your computer.  Select the hard disk you wish to use for installation."));

    QVBoxLayout *layout = new QVBoxLayout;
    layout->addSpacing(10);
    layout->addWidget(descLabel);
    layout->addSpacing(10);
    layout->addWidget(diskChooser);
    setLayout(layout);
}

void PartitionDiskPage::initializePage() {
    for(auto disk : horizonWizard()->disks) {
        QString name{QString("%1 (%2)\n%3 MB available of %4 MB")
                    .arg(QString::fromStdString(disk.model()))
                    .arg(QString::fromStdString(disk.name()))
                    .arg(disk.contiguous_block()).arg(disk.total_size())};
        QListWidgetItem *item = new QListWidgetItem(iconForDisk(disk), name, diskChooser);
        item->setToolTip(QString::fromStdString(disk.dev_path()));
        item->setData(Qt::UserRole, QString::fromStdString(disk.node()));
    }

    if(horizonWizard()->disks.size() == 0) {
        QLabel *exclamation, *explanation;
        exclamation = new QLabel;
        exclamation->setPixmap(QIcon::fromTheme("dialog-warning").pixmap(QSize(128, 128)));
        explanation = new QLabel(tr("<p><strong>No disks have been found on your computer.</strong></p><p>Consult the Installation Handbook for more information.</p>"));
        explanation->setAlignment(Qt::AlignCenter);
        explanation->setTextFormat(Qt::RichText);
        diskChooser->setHidden(true);
        QVBoxLayout *myLayout = dynamic_cast<QVBoxLayout *>(layout());
        myLayout->addStretch();
        myLayout->addWidget(exclamation, 0, Qt::AlignCenter);
        myLayout->addWidget(explanation, 0, Qt::AlignCenter);
        myLayout->addStretch();
    }
}

bool PartitionDiskPage::isComplete() const {
    return diskChooser->currentIndex().row() != -1;
}
