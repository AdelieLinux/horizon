/*
 * pkgmodel.cc - Implementation of the custom package selection model classes
 * horizon-qt5, the Qt 5 user interface for
 * Project Horizon
 *
 * Copyright (c) 2019 Adélie Linux and contributors.  All rights reserved.
 * This code is licensed under the AGPL 3.0 license, as noted in the
 * LICENSE-code file in the root directory of this repository.
 *
 * SPDX-License-Identifier: AGPL-3.0-only
 */

#include "pkgmodel.hh"
#include <QFile>
#include <QIcon>
#include <QMessageBox>

PkgItem::PkgItem(PkgItem *parent, QString friendly, QString internal,
                 QString description, QString icon) : _parent(parent),
                                                      _friendly(friendly),
                                                      _internal(internal),
                                                      _desc(description) {
    if(icon.isEmpty()) {
        _icon = internal;
    } else {
        _icon = icon;
    }
}

PkgItem::~PkgItem() {
    qDeleteAll(_children);
}

void PkgItem::addChild(PkgItem *pkg) {
    _children.append(pkg);
}

PkgItem *PkgItem::child(int row) {
    if(row < 0 || row > _children.size()) {
        return nullptr;
    }

    return _children.at(row);
}

int PkgItem::childCount() const {
    return _children.size();
}

QVariant PkgItem::data(int column, int role) {
    if(column != 0) {
        return QVariant();
    }

    switch(role) {
    case Qt::DisplayRole:
        return QVariant(_friendly);
    case Qt::DecorationRole:
        return QVariant(QIcon::fromTheme(_icon));
    case Qt::StatusTipRole:
    case Qt::WhatsThisRole:
        return QVariant(_desc);
    case Qt::UserRole:
        return QVariant(_internal);
    }

    return QVariant();
}

QStringList PkgItem::internalNames() const {
    QStringList list;

    if(!_internal.isEmpty()) {
        list.append(_internal);
    }

    for(auto &child : _children) {
        list.append(child->internalNames());
    }

    return list;
}

PkgItem *PkgItem::parent() {
    return _parent;
}

int PkgItem::row() const {
    if(_parent == nullptr) {
        return 0;
    }

    return _parent->_children.indexOf(const_cast<PkgItem *>(this));
}


PkgItemModel::PkgItemModel(QStringList *pkgList, QObject *parent)
    : QAbstractItemModel(parent), _packages(pkgList) {
    _root = new PkgItem(nullptr, "Root Item", "root", "The root of the tree.  Never seen.");

    loadPackages();
}

PkgItemModel::~PkgItemModel() {
    delete _root;
}

int PkgItemModel::columnCount(const QModelIndex &) const {
    return 1;
}

QVariant PkgItemModel::data(const QModelIndex &index, int role) const {
    if(!index.isValid()) {
        return QVariant();
    }

    PkgItem *item = static_cast<PkgItem *>(index.internalPointer());
    if(role == Qt::CheckStateRole && index.column() == 0) {
        QStringList pkgs = item->internalNames();
        if(_packages && !pkgs.isEmpty()) {
            bool all = true, any = false;
            for(auto &pkg : pkgs) {
                if(_packages->contains(pkg)) {
                    any = true;
                } else {
                    all = false;
                }
            }

            if(all) return Qt::Checked;
            if(any) return Qt::PartiallyChecked;
        }
        return Qt::Unchecked;
    }

    return item->data(index.column(), role);
}

Qt::ItemFlags PkgItemModel::flags(const QModelIndex &index) const {
    if(!index.isValid()) {
        return Qt::NoItemFlags;
    }

    Qt::ItemFlags flags = Qt::ItemIsEnabled;
    if(index.column() == 0) {
        flags |= Qt::ItemIsUserCheckable | Qt::ItemIsSelectable | Qt::ItemIsAutoTristate;
    }

    return flags;
}

QVariant PkgItemModel::headerData(int, Qt::Orientation, int) const {
    return QVariant();
}

QModelIndex PkgItemModel::index(int row, int column,
                                const QModelIndex &parent) const {
    if(!hasIndex(row, column, parent)) {
        return QModelIndex();
    }

    PkgItem *p;

    if(parent.isValid()) {
        p = static_cast<PkgItem *>(parent.internalPointer());
    } else {
        p = _root;
    }

    PkgItem *item = p->child(row);
    if(item) {
        return createIndex(row, column, item);
    } else {
        return QModelIndex();
    }
}

QModelIndex PkgItemModel::parent(const QModelIndex &child) const {
    if(!child.isValid()) {
        return QModelIndex();
    }

    PkgItem *item = static_cast<PkgItem *>(child.internalPointer());
    PkgItem *parent = item->parent();

    if(parent == _root) {
        return QModelIndex();
    } else {
        return createIndex(parent->row(), 0, parent);
    }
}

int PkgItemModel::rowCount(const QModelIndex &parent) const {
    if(parent.column() > 0) {
        return 0;
    }

    if(parent.isValid()) {
        PkgItem *item = static_cast<PkgItem *>(parent.internalPointer());
        return item->childCount();
    } else {
        return _root->childCount();
    }
}

bool PkgItemModel::setData(const QModelIndex &index, const QVariant &value,
                           int role) {
    if(!index.isValid() || role != Qt::CheckStateRole) {
        return false;
    }

    PkgItem *item = static_cast<PkgItem *>(index.internalPointer());
    if(item) {
        QStringList pkgs = item->internalNames();
        if(value.toBool()) {
            _packages->append(pkgs);
        } else {
            for(auto &pkg : pkgs) {
                _packages->removeAll(pkg);
            }
        }
        emit dataChanged(index, index, {Qt::CheckStateRole});
        if(item->childCount() > 0) {
            emit dataChanged(createIndex(0, 0, item),
                             createIndex(item->childCount(), 0, item),
                             {Qt::CheckStateRole});
        }

        PkgItem *parent = item;
        while(parent->parent() != _root) {
            parent = parent->parent();
            QModelIndex pidx = createIndex(parent->row(), 0, parent);
            emit dataChanged(pidx, pidx, {Qt::CheckStateRole});
        }
    }

    return true;
}

void PkgItemModel::setPackageList(QStringList *pkgList) {
    _packages = pkgList;
}

void PkgItemModel::loadPackages() {
    QFile myList(":/packages.txt");
    if(!myList.open(QFile::ReadOnly)) {
        QMessageBox::critical(nullptr, tr("Could Not Load Software List"),
                              tr("An issue occurred loading the available software list."));
        return;
    }

    QByteArray raw_pkgs = myList.readAll();
    QList<QByteArray> pkgs = raw_pkgs.split('\n');
    int lastIndent = 0;
    PkgItem *temp = new PkgItem(_root, "Placeholder", "Placeholder", "Used for loading only");
    PkgItem *last = temp;

    for(auto &pkg : pkgs) {
        int indent = 0;

        if(pkg.isEmpty()) continue;

        while(pkg.size() > 1 && pkg.at(0) == ' ') {
            indent++;
            pkg.remove(0, 1);
        }

        /* minimum valid size: A\tb\tC */
        if(pkg.size() < 6) continue;

        PkgItem *myParent;

        if(indent == lastIndent) {
            /* We're siblings of the last item. */
            myParent = last->parent();
        } else if(indent < lastIndent) {
            /* Traverse up until we're equal */
            while(lastIndent-- > indent) {
                last = last->parent();
            }
            myParent = last->parent();
        } else /*(indent > lastIndent)*/ {
            /* We're children of the last item. */
            myParent = last;
        }

        QList<QByteArray> fields = pkg.split('\t');
        /* at least friendly, internal, desc required */
        if(fields.size() < 3) continue;
        last = new PkgItem(myParent, fields[0], fields[1], fields[2],
                (fields.size() == 4 ? fields[3] : fields[1]));
        myParent->addChild(last);
        lastIndent = indent;
    }
    delete temp;
}
