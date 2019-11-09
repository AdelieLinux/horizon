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

void HorizonWizardPage::loadWatermark(string page) {
    QPixmap pixmap;
    qreal pixelRatio = 0;
    string path = ":/wizard_pixmaps/resources/";
    path += page;
    path += "-";

    if(window()->devicePixelRatioF() == 2.0) {
        path += "high";
        pixelRatio = 2.0;
    } else if(window()->devicePixelRatioF() == 1.0) {
        path += "low";
        pixelRatio = 1.0;
    } else {
        path += "high";
    }

    path += ".png";

    pixmap = QPixmap(path.c_str());

    // Handle cases where ratio is not exactly 1.0 or 2.0
    // Wizard machinary automatically uses FastTransformation, which is
    // ugly as sin.
    if(pixelRatio > 1.0) {
        qreal width = 232 * window()->devicePixelRatioF();
        qreal height = 380 * window()->devicePixelRatioF();
        QSize newSize = QSize(width, height);

        pixmap = pixmap.scaled(newSize, Qt::KeepAspectRatio,
                               Qt::SmoothTransformation);
        pixmap.setDevicePixelRatio(window()->devicePixelRatioF());
    } else {
        pixmap.setDevicePixelRatio(pixelRatio);
    }

    setPixmap(QWizard::WatermarkPixmap, pixmap);
}

HorizonWizard *HorizonWizardPage::horizonWizard() const {
    return dynamic_cast<HorizonWizard *>(this->wizard());
}
