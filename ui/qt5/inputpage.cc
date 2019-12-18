/*
 * inputpage.cc - Implementation of the UI.Input page
 * horizon-qt5, the Qt 5 user interface for
 * Project Horizon
 *
 * Copyright (c) 2019 Adélie Linux and contributors.  All rights reserved.
 * This code is licensed under the AGPL 3.0 license, as noted in the
 * LICENSE-code file in the root directory of this repository.
 *
 * SPDX-License-Identifier: AGPL-3.0-only
 */

#include "inputpage.hh"

#include <QDebug>
#include <QLabel>
#include <QVBoxLayout>

#include "util/keymaps.hh"

#ifdef HAS_INSTALL_ENV
#include <QProcess>
#include <QPushButton>
#include <QLineEdit>
#include <X11/XKBlib.h>
#include <X11/extensions/XKBrules.h>
#endif  /* HAS_INSTALL_ENV */

InputPage::InputPage(QWidget *parent) : HorizonWizardPage(parent) {
    QLabel *descLabel;
    QVBoxLayout *layout;

    loadWatermark("intro");
    setTitle(tr("Keyboard Layout"));

    descLabel = new QLabel(tr("Choose the layout of your keyboard."));
    descLabel->setWordWrap(true);

    /* REQ: UI.Input.LayoutList */
    layoutList = new QListWidget(this);
    layoutList->setSelectionMode(QAbstractItemView::SingleSelection);
    layoutList->setWhatsThis(tr("This is a list of keyboard layouts.  Select one to choose the layout of the keyboard you will be using on your Adélie Linux computer."));
    for(auto &map : valid_keymaps) {
        QIcon myIcon = QIcon::fromTheme("input-keyboard");
        new QListWidgetItem(myIcon, map.c_str(), layoutList);
    }

    registerField("keymap*", layoutList);

#ifdef HAS_INSTALL_ENV
    /* REQ: UI.Input.ChooseLayout */
    QObject::connect(layoutList, &QListWidget::currentItemChanged,
                     [](QListWidgetItem *current, QListWidgetItem *) {
        if(current == nullptr) return;

        QProcess setxkbmap;
        setxkbmap.execute("setxkbmap", {current->text()});
    });

    /* REQ: UI.Input.Test */
    QLineEdit *testArea = new QLineEdit(this);
    testArea->setPlaceholderText(tr("Type here to test the keyboard layout you've selected."));
    testArea->setWhatsThis(tr("By typing into this box, you can ensure the layout you've selected is the correct one."));
#endif  /* HAS_INSTALL_ENV */

    layout = new QVBoxLayout(this);
    layout->addWidget(descLabel);
    layout->addSpacing(20);
    layout->addWidget(layoutList);
#ifdef HAS_INSTALL_ENV
    layout->addStretch();
    layout->addWidget(testArea);
#endif  /* HAS_INSTALL_ENV */

    setLayout(layout);
}

void InputPage::initializePage() {
#ifdef HAS_INSTALL_ENV
    /* Select the current keyboard layout, if available. */
    Display *dpy = XOpenDisplay(nullptr);
    if(dpy != nullptr) {
        XkbRF_VarDefsRec vardefs{};
        XkbRF_GetNamesProp(dpy, nullptr, &vardefs);
        QList<QListWidgetItem *> items = layoutList->findItems(vardefs.layout, Qt::MatchExactly);
        if(!items.empty()) layoutList->setCurrentItem(items.at(0));
        free(vardefs.model);
        free(vardefs.layout);
        free(vardefs.variant);
        free(vardefs.options);
        XCloseDisplay(dpy);
    }
#endif  /* HAS_INSTALL_ENV */
}
