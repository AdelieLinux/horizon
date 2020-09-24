/*
 * netsimplewifipage.cc - Implementation of the UI.Network.Wireless page
 * horizon-qt5, the Qt 5 user interface for
 * Project Horizon
 *
 * Copyright (c) 2019-2020 Adélie Linux and contributors.  All rights reserved.
 * This code is licensed under the AGPL 3.0 license, as noted in the
 * LICENSE-code file in the root directory of this repository.
 *
 * SPDX-License-Identifier: AGPL-3.0-only
 */

#include "netsimplewifipage.hh"
#include "customwifidialog.hh"
#include "netdhcppage.hh"

#include <assert.h>
#include <sstream>
#include <QAction>
#include <QHBoxLayout>
#include <QVBoxLayout>

#ifdef HAS_INSTALL_ENV
#   include <QApplication>
#   include <QMessageBox>
#   include <QProgressBar>
int scanResults(wpactrl_t *control, char const *s, size_t len, void *page, tain_t *) {
    NetworkSimpleWirelessPage *our_page = reinterpret_cast<NetworkSimpleWirelessPage *>(page);
    return our_page->processScan(control, s, len);
}
#endif  /* HAS_INSTALL_ENV */


/*! Holds AP flags like WPA2, etc */
#define FLAGS_ROLE      Qt::UserRole
/*! Holds the RSSI/signal strength */
#define SIGNAL_ROLE     Qt::UserRole + 1


NetworkSimpleWirelessPage::NetworkSimpleWirelessPage(QWidget *parent)
    : HorizonWizardPage(parent)
#ifdef HAS_INSTALL_ENV
    , control(WPACTRL_ZERO), associated(false)
#endif  /* HAS_INSTALL_ENV */
    {
    QVBoxLayout *layout = new QVBoxLayout;
    QHBoxLayout *buttonLayout = new QHBoxLayout;

    loadWatermark("network");
    setTitle(tr("Select Your Network"));

    statusLabel = new QLabel(tr("Scanning for networks..."));
    statusLabel->setAlignment(Qt::AlignCenter);
    statusLabel->setWordWrap(true);

    addNetButton = new QPushButton(tr("&Unlisted Network..."));
    addNetButton->setIcon(QIcon::fromTheme("list-add"));
    connect(addNetButton, &QPushButton::clicked, [=] {
        CustomWiFiDialog d;
        if(d.exec() == QDialog::Accepted) {
            QListWidgetItem *netitem = new QListWidgetItem;
            netitem->setText(d.networkName());
            netitem->setIcon(QIcon::fromTheme("network-wireless-signal-none"));
            netitem->setToolTip(tr("Frequency: Unknown"));
            netitem->setData(FLAGS_ROLE, d.flags());
            netitem->setData(SIGNAL_ROLE, 2);

            /* push to top */
            ssidListView->insertItem(0, netitem);
        }
    });

    buttonLayout->addSpacing(10);
    buttonLayout->addWidget(addNetButton, 0, Qt::AlignLeft);

    ssidListView = new QListWidget;
    connect(ssidListView, &QListWidget::currentItemChanged,
            this, &NetworkSimpleWirelessPage::networkChosen);

#ifdef HAS_INSTALL_ENV
    rescanButton = new QPushButton(tr("&Rescan Networks"));
    connect(rescanButton, &QPushButton::clicked, [=](void) { doScan(); });

    buttonLayout->addWidget(rescanButton, 0, Qt::AlignRight);
    buttonLayout->addSpacing(10);

    exchange_item.filter = "CTRL-EVENT-SCAN-RESULTS";
    exchange_item.cb = &scanResults;
    notify = nullptr;
    connNotify = nullptr;
    dialog = nullptr;
#endif  /* HAS_INSTALL_ENV */

    buttonLayout->addSpacing(10);
    passphrase = new QLineEdit(this);
    connect(passphrase, &QLineEdit::textChanged,
            this, &NetworkSimpleWirelessPage::completeChanged);
    passphrase->setEchoMode(QLineEdit::Password);
    passphrase->setMinimumWidth(255);
    passphrase->hide();
    QAction *togglePass = passphrase->addAction(QIcon::fromTheme("visibility"),
                                                QLineEdit::TrailingPosition);
    togglePass->setToolTip(tr("Show the passphrase"));
    togglePass->setData(true);
    connect(togglePass, &QAction::triggered,
            [=](void) {
        if(togglePass->data().toBool() == true) {
            togglePass->setData(false);
            togglePass->setIcon(QIcon::fromTheme("hint"));
            togglePass->setToolTip(tr("Hide the passphrase"));
            passphrase->setEchoMode(QLineEdit::Normal);
        } else {
            togglePass->setData(true);
            togglePass->setIcon(QIcon::fromTheme("visibility"));
            togglePass->setToolTip(tr("Show the passphrase"));
            passphrase->setEchoMode(QLineEdit::Password);
        }
    });
    passLabel = new QLabel;
    passLabel->setBuddy(passphrase);

    layout->addWidget(statusLabel);
    layout->addSpacing(10);
    layout->addWidget(ssidListView, 0, Qt::AlignCenter);
    layout->addLayout(buttonLayout);
    layout->addSpacing(10);
    layout->addWidget(passLabel, 0, Qt::AlignCenter);
    layout->addWidget(passphrase, 0, Qt::AlignCenter);
    setLayout(layout);
}

NetworkSimpleWirelessPage::~NetworkSimpleWirelessPage() {
#ifdef HAS_INSTALL_ENV
    wpactrl_end(&control);
#endif  /* HAS_INSTALL_ENV */
}

#ifdef HAS_INSTALL_ENV
void NetworkSimpleWirelessPage::scanDone(QString message) {
    rescanButton->setEnabled(true);
    statusLabel->setText(message);
}
#endif  /* HAS_INSTALL_ENV */

void NetworkSimpleWirelessPage::networkChosen(QListWidgetItem *current,
                                              QListWidgetItem *) {
    passphrase->clear();
    passphrase->hide();
    passLabel->setText("");

    if(current == nullptr) {
        emit completeChanged();
        return;
    }

    QStringList flags = current->data(Qt::UserRole).toStringList();
    if(flags.length() == 0) {
        goto done;
    }

    for(auto &flag : flags) {
        if(flag.startsWith("WPA-EAP") || flag.startsWith("WPA2-EAP")) {
            passphrase->setEnabled(false);
            passphrase->setPlaceholderText(tr("WPA Enterprise networks are not supported in this release of Horizon."));
            passphrase->show();
            passLabel->setText(tr("Enter network credentials:"));
            goto done;
        }

        if(flag.startsWith("WPA-PSK") || flag.startsWith("WPA2-PSK")) {
            passphrase->setEnabled(true);
            passphrase->setPlaceholderText(tr("WPA Passphrase"));
            passphrase->show();
            passLabel->setText(tr("Enter network password:"));
            goto done;
        }

        if(flag.startsWith("WEP")) {
            passphrase->setEnabled(true);
            passphrase->setPlaceholderText(tr("WEP Passphrase"));
            passphrase->show();
            passLabel->setText(tr("Enter network password:"));
            goto done;
        }
    }

done:
    emit completeChanged();
    return;
}

#ifdef HAS_INSTALL_ENV
void NetworkSimpleWirelessPage::doScan() {
    ssidListView->clear();
    rescanButton->setEnabled(false);
    statusLabel->setText(tr("Scanning for networks..."));

    tain_t deadline;
    wparesponse_t response;
    std::string suppsock = "/var/run/wpa_supplicant/" +
            horizonWizard()->chosen_auto_iface;

    tain_now_g();
    wpactrl_end(&control);
    if(!wpactrl_start_g(&control, suppsock.c_str(), 2000)) {
        rescanButton->setEnabled(false);
        int error = errno;
        statusLabel->setText(tr("Couldn't communicate with wireless system (Code %1)").arg(error));
        return;
    }

    response = wpactrl_command_g(&control, "SCAN");
    if(response != WPA_OK && response != WPA_FAILBUSY) {
        scanDone(tr("Couldn't scan for wireless networks (Code %1)").arg(response));
        return;
    }

    tain_from_millisecs(&deadline, 15000);
    tain_add_g(&deadline, &deadline);
    wpactrl_xchg_init(&exchange, &exchange_item, 1, &deadline, this);
    if(!wpactrl_xchg_start(&control, &exchange)) {
        scanDone(tr("Failed to scan for wireless networks."));
        return;
    }

    if(notify != nullptr) {
        notify->setEnabled(false);
        notify->deleteLater();
        notify = nullptr;
    }

    notify = new QSocketNotifier(wpactrl_fd(&control), QSocketNotifier::Read, this);
    connect(notify, &QSocketNotifier::activated, [=](int) {
        QString status;

        tain_now_g();
        if(wpactrl_xchg_timeout_g(&control, &exchange)) {
            status = tr("Network scan timed out.");
        } else {
            if(wpactrl_update(&control) < 0) {
                status = tr("Issue communicating with wireless system.");
            } else {
                int code = wpactrl_xchg_event_g(&control, &exchange);
                if(code == -2) {
                    /* if the callback is what failed, we already set status */
                    status = statusLabel->text();
                } else if(code < 0) {
                    /* non-callback failure */
                    status = tr("Issue processing scanned networks (Code %1)")
                            .arg(code);
                } else if(code == 0) {
                    /* Not finished yet, so don't do anything. */
                    return;
                } else {
                    status = tr("Select your wireless network.");
                }
            }
        }
        notify->setEnabled(false);
        notify->deleteLater();
        notify = nullptr;
        statusLabel->setText(status);
        rescanButton->setEnabled(true);
        return;
    });
    notify->setEnabled(true);
}
#endif  /* HAS_INSTALL_ENV */

void NetworkSimpleWirelessPage::initializePage() {
#ifdef HAS_INSTALL_ENV
    doScan();
#endif  /* HAS_INSTALL_ENV */
}

bool NetworkSimpleWirelessPage::isComplete() const {
    if(ssidListView->currentRow() == -1) {
        return false;
    }

    return (passphrase->isHidden() || passphrase->text().size() > 0);
}

int NetworkSimpleWirelessPage::nextId() const {
    return HorizonWizard::Page_Network_DHCP;
}

#ifdef HAS_INSTALL_ENV
void NetworkSimpleWirelessPage::associate() {
    QList<QListWidgetItem *> items = ssidListView->selectedItems();
    if(items.size() == 0) {
        return;
    }

    if(dialog != nullptr) dialog->deleteLater();
    dialog = new QProgressDialog(this);
    dialog->setCancelButton(nullptr);

    QProgressBar *bar = new QProgressBar;
    bar->setRange(0, 0);
    bar->setFixedSize(QSize(150, 24));
    dialog->setBar(bar);
    dialog->setLabelText(tr("Connecting..."));
    dialog->setMinimumDuration(1);

    dialog->show();
    QApplication::processEvents(QEventLoop::AllEvents, 1000);

    if(connNotify != nullptr) {
        connNotify->setEnabled(false);
        connNotify->deleteLater();
        connNotify = nullptr;
    }

    wpactrl_filter_add(&control, "CTRL-EVENT-ASSOC-REJECT");
    wpactrl_filter_add(&control, "CTRL-EVENT-AUTH-REJECT");
    wpactrl_filter_add(&control, "CTRL-EVENT-CONNECTED");

    connNotify = new QSocketNotifier(wpactrl_fd(&control), QSocketNotifier::Read, this);
    connect(connNotify, &QSocketNotifier::activated,
            this, &NetworkSimpleWirelessPage::processAssociateMessage);

    const char *ssid, *pass;
    std::string password, network;

    if(passphrase->isHidden()) {
        pass = nullptr;
    } else {
        password = ("\"" + passphrase->text().toStdString() + "\"");
        pass = password.c_str();
    }
    network = ("\"" + items[0]->text().toStdString() + "\"");
    ssid = network.c_str();

    tain_now_g();
    if(wpactrl_associate_g(&control, ssid, pass) == 0) {
        dialog->hide();
        QMessageBox::critical(this, tr("Could Not Connect"),
                              tr("An issue occurred connecting to the specified wireless network.  "
                                 "Ensure your passphrase is correct and try again."));
    } else {
        dialog->setLabelText(tr("Associating..."));
    }

    return;
}

void NetworkSimpleWirelessPage::processAssociateMessage(int) {
    QString error;

    if(wpactrl_update(&control) < 0) {
        dialog->setLabelText(tr("Issue communicating with wireless subsystem."));
    } else {
        char *raw_msg = wpactrl_msg(&control);
        if(raw_msg == nullptr) {
            return;
        }
        QString msg(raw_msg);
        msg = msg.remove(0, 3);

        wpactrl_ackmsg(&control);

        if(msg.startsWith("CTRL-EVENT-CONNECTED")) {
            /* Happy day! */
            dialog->setRange(0, 1);
            dialog->setValue(1);
            dialog->setLabelText(tr("Connected."));
            associated = true;
            horizonWizard()->next();
        } else if(msg.startsWith("CTRL-EVENT-ASSOC-REJECT")) {
            error = tr("There was an issue connecting to your wireless network.  "
                       "You may need to move closer to your wireless gateway, or reset your hardware and try again.\n\n"
                       "Technical details: %1").arg(msg);
        } else if(msg.startsWith("CTRL-EVENT-AUTH-REJECT")) {
            error = tr("There was an issue connecting to your wireless network.  "
                       "Ensure your passphrase is correct and try again.\n\n"
                       "Technical details: %1").arg(msg);
        } else {
            /* unknown message. */
            return;
        }
    }

    connNotify->setEnabled(false);
    connNotify->deleteLater();
    connNotify = nullptr;

    if(!associated) {
        dialog->hide();
        QMessageBox::critical(this, tr("Could Not Connect"), error);
    }

    return;
}

int NetworkSimpleWirelessPage::processScan(wpactrl_t *c, const char *, size_t) {
    assert(c == &control);
    std::vector<QListWidgetItem *> netitems;

    stralloc buf = STRALLOC_ZERO;

    errno = 0;
    if(!wpactrl_querysa_g(&control, "SCAN_RESULTS", &buf)) {
        if(errno == EMSGSIZE) {
            statusLabel->setText(tr("Scan failed: Out of memory"));
            return 0;
        } else {
            statusLabel->setText(tr("Scan failed (Code %1)").arg(errno));
            return 0;
        }
    }

    genalloc nets = GENALLOC_ZERO;
    stralloc netstr = STRALLOC_ZERO;
    errno = 0;

    if(wpactrl_scan_parse(buf.s, buf.len, &nets, &netstr) != 1) {
        statusLabel->setText(tr("Network listing failed (Code %1)").arg(errno));
        return 0;
    }

    wpactrl_scanres_t *netarray = genalloc_s(wpactrl_scanres_t, &nets);
    for(size_t net = 0; net < genalloc_len(wpactrl_scanres_t, &nets); net++) {
        wpactrl_scanres_t network = netarray[net];
        std::string ssid(netstr.s + network.ssid_start, network.ssid_len);
        std::string flags(netstr.s + network.flags_start, network.flags_len);

        /* Don't bother with empty SSIDs. */
        if(ssid.empty()) continue;
        /* since this *is* destructive, we don't run it on our actual SSID */
        if(QString::fromStdString(ssid)
                .remove("\\x00", Qt::CaseSensitive).size() == 0) continue;

        QIcon icon;
        if(network.signal_level < -90) {
            icon = QIcon::fromTheme("network-wireless-signal-none");
        } else if(network.signal_level < -80) {
            icon = QIcon::fromTheme("network-wireless-signal-weak");
        } else if(network.signal_level < -67) {
            icon = QIcon::fromTheme("network-wireless-signal-ok");
        } else if(network.signal_level < -50) {
            icon = QIcon::fromTheme("network-wireless-signal-good");
        } else {
            icon = QIcon::fromTheme("network-wireless-signal-excellent");
        }

        QListWidgetItem *netitem = new QListWidgetItem;
        netitem->setText(QString::fromStdString(ssid));
        netitem->setIcon(icon);
        netitem->setToolTip(tr("Frequency: %1 MHz\nBSSID: %2\nRSSI: %3")
                            .arg(network.frequency)
                            .arg(fromMacAddress(network.bssid))
                            .arg(network.signal_level));
        QString zero(QString::fromStdString(std::string("\0", 1)));
        netitem->setData(FLAGS_ROLE, QString::fromStdString(flags).split(zero));
        netitem->setData(SIGNAL_ROLE, network.signal_level);
        netitems.push_back(netitem);
    }

    std::sort(netitems.begin(), netitems.end(),
              [](QListWidgetItem *left, QListWidgetItem *right) {
        return left->data(SIGNAL_ROLE).toInt() >
               right->data(SIGNAL_ROLE).toInt();
    });

    for(auto &item : netitems) {
        ssidListView->addItem(item);
    }

    stralloc_free(&netstr);
    genalloc_free(wpactrl_scanres_t, &nets);
    stralloc_free(&buf);

    return 1;
}
#endif  /* HAS_INSTALL_ENV */

bool NetworkSimpleWirelessPage::validatePage() {
    /* What a hack!
     *
     * Independent Pages means the DHCP page is never cleaned, even when Back
     * is chosen.  So, we have to do it from here. */
    horizonWizard()->removePage(HorizonWizard::Page_Network_DHCP);
    horizonWizard()->setPage(HorizonWizard::Page_Network_DHCP, new NetDHCPPage);

#ifdef HAS_INSTALL_ENV
    if(associated) return true;

    associate();
    return false;
#else  /* !HAS_INSTALL_ENV */
    return true;
#endif  /* HAS_INSTALL_ENV */
}
