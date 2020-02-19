/*
 * partitionmanualpage.hh - Definition of the UI.Partition.Install.Manual page
 * horizon-qt5, the Qt 5 user interface for
 * Project Horizon
 *
 * Copyright (c) 2020 Adélie Linux and contributors.  All rights reserved.
 * This code is licensed under the AGPL 3.0 license, as noted in the
 * LICENSE-code file in the root directory of this repository.
 *
 * SPDX-License-Identifier: AGPL-3.0-only
 */

#ifndef PARTITIONMANUALPAGE_HH
#define PARTITIONMANUALPAGE_HH

#include "horizonwizardpage.hh"

#ifndef HAS_INSTALL_ENV
#   include <QTextEdit>
#endif  /* !HAS_INSTALL_ENV */

class PartitionManualPage : public HorizonWizardPage {
public:
    PartitionManualPage(QWidget *parent = nullptr);
#ifndef HAS_INSTALL_ENV
    bool isComplete() const override;
    int nextId() const override;
private:
    QTextEdit *partitionEdit;
#endif  /* !HAS_INSTALL_ENV */
};

#endif  /* !PARTITIONMANUALPAGE_HH */
