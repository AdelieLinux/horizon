/*
 * horizonwizardpage.cc - Implementation of our custom QWizardPage subclass
 * horizon-qt5, the Qt 5 user interface for
 * Project Horizon
 *
 * Copyright (c) 2019 Adélie Linux and contributors.  All rights reserved.
 * This code is licensed under the AGPL 3.0 license, as noted in the
 * LICENSE-code file in the root directory of this repository.
 *
 * SPDX-License-Identifier: AGPL-3.0-only
 */

#include "horizonwizardpage.hh"

using std::string;

QPixmap HorizonWizardPage::loadDPIAwarePixmap(string pixmap, string type) {
    string path = ":/wizard_pixmaps/resources/";
    path += pixmap;
    path += "-";

    if(window()->devicePixelRatioF() <= 1.0) {
        path += "low";
    } else {
        path += "high";
    }

    path += type;
    return QPixmap(path.c_str());
}

void HorizonWizardPage::loadWatermark(string page) {
    QPixmap pixmap = loadDPIAwarePixmap(page);
    qreal pixelRatio = window()->devicePixelRatioF();

    // Handle cases where ratio is not exactly 1.0 or 2.0
    // Wizard machinary automatically uses FastTransformation, which is
    // ugly as sin.
    if(pixelRatio > 1.0) {
        qreal width = 232 * pixelRatio;
        qreal height = 380 * pixelRatio;
        QSize newSize = QSize(width, height);

        pixmap = pixmap.scaled(newSize, Qt::KeepAspectRatio,
                               Qt::SmoothTransformation);
    }
    pixmap.setDevicePixelRatio(pixelRatio);

    setPixmap(QWizard::WatermarkPixmap, pixmap);
}

HorizonWizard *HorizonWizardPage::horizonWizard() const {
    return dynamic_cast<HorizonWizard *>(this->wizard());
}
