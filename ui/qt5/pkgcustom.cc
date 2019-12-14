/*
 * pkgcustom.cc - Implementation of the UI.Packages.Custom page
 * horizon-qt5, the Qt 5 user interface for
 * Project Horizon
 *
 * Copyright (c) 2019 Adélie Linux and contributors.  All rights reserved.
 * This code is licensed under the AGPL 3.0 license, as noted in the
 * LICENSE-code file in the root directory of this repository.
 *
 * SPDX-License-Identifier: AGPL-3.0-only
 */

#include "pkgcustom.hh"

#include <QGroupBox>
#include <QTreeView>
#include <QVBoxLayout>

PkgCustomPage::PkgCustomPage(QWidget *parent) : HorizonWizardPage(parent) {
    setTitle(tr("Software Selection"));
    loadWatermark("software");

    QLabel *descLabel = new QLabel(tr(
        "Choose the software you want to install on this computer.\n\n"
        "You may select any category or software package to read a brief description of it."));
    descLabel->setWordWrap(true);

    QTreeView *pkgTree = new QTreeView;
    model = new PkgItemModel(nullptr, this);
    pkgTree->setHeaderHidden(true);
    pkgTree->setModel(model);
    pkgTree->setUniformRowHeights(true);

    QItemSelectionModel *selModel = pkgTree->selectionModel();
    connect(selModel, &QItemSelectionModel::currentChanged, [=](const QModelIndex &current){
        QVariant data = model->data(current, Qt::StatusTipRole);
        if(data.isValid()) {
            pkgDescLabel->setText(data.toString());
        }
    });

    pkgDescLabel = new QLabel(tr("Select an item to view its description.\n\n "));
    pkgDescLabel->setWordWrap(true);
    QGroupBox *descBox = new QGroupBox("Description");
    QVBoxLayout *descLayout = new QVBoxLayout;
    descLayout->addWidget(pkgDescLabel);
    descBox->setLayout(descLayout);

    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(descLabel);
    layout->addWidget(pkgTree);
    layout->addWidget(descBox);
    setLayout(layout);
}

void PkgCustomPage::initializePage() {
    model->setPackageList(&(horizonWizard()->packages));
}
