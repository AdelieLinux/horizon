/*
 * pkgmodel.hh - Definition of the custom package selection model classes
 * horizon-qt5, the Qt 5 user interface for
 * Project Horizon
 *
 * Copyright (c) 2019 Adélie Linux and contributors.  All rights reserved.
 * This code is licensed under the AGPL 3.0 license, as noted in the
 * LICENSE-code file in the root directory of this repository.
 *
 * SPDX-License-Identifier: AGPL-3.0-only
 */

#ifndef PKGMODEL_HH
#define PKGMODEL_HH

#include <QAbstractItemModel>

class PkgItem {
public:
    explicit PkgItem(PkgItem *parent, QString friendly, QString internal,
                     QString description, QString icon = "");
    ~PkgItem();

    void addChild(PkgItem *pkg);
    PkgItem *child(int row);
    int childCount() const;
    QVariant data(int column, int role);
    QStringList internalNames() const;
    int row() const;
    PkgItem *parent();

private:
    QVector<PkgItem *> _children;
    PkgItem *_parent;
    /*! The "friendly" or displayed name for this item. */
    QString _friendly;
    /*! The internal name.  Typically the APK package name. */
    QString _internal;
    /*! The description of the item. */
    QString _desc;
    /*! The icon to display for this item.  Defaults to internal. */
    QString _icon;
};

class PkgItemModel : public QAbstractItemModel {
Q_OBJECT
public:
    explicit PkgItemModel(QStringList *pkgList = nullptr,
                          QObject *parent = nullptr);
    ~PkgItemModel();

    QVariant data(const QModelIndex &index,
                  int role = Qt::DisplayRole) const override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;
    QVariant headerData(int section, Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const override;
    QModelIndex index(int row, int column,
                      const QModelIndex &parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex &child) const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    bool setData(const QModelIndex &index, const QVariant &value,
                 int role = Qt::EditRole) override;
    void setPackageList(QStringList *pkgList);

private:
    void loadPackages();
    /*! The list of packages that will be installed.
     *  Determines which packages are ticked. */
    QStringList *_packages;
    /*! The special root item, which all other items have as an eventual
     *  parent. */
    PkgItem *_root;
};

#endif  /* !PKGMODEL_HH */
