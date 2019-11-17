/*
 * netsimplewifipage.cc - Implementation of the UI.Network.Wireless page
 * horizon-qt5, the Qt 5 user interface for
 * Project Horizon
 *
 * Copyright (c) 2019 Adélie Linux and contributors.  All rights reserved.
 * This code is licensed under the AGPL 3.0 license, as noted in the
 * LICENSE-code file in the root directory of this repository.
 *
 * SPDX-License-Identifier: AGPL-3.0-only
 */

#include "netsimplewifipage.hh"
#include "netdhcppage.hh"

#include <assert.h>
#include <sstream>
#include <QVBoxLayout>

#ifdef HAS_INSTALL_ENV
int scanResults(wpactrl_t *control, char const *s, size_t len, void *page, tain_t *) {
    NetworkSimpleWirelessPage *our_page = reinterpret_cast<NetworkSimpleWirelessPage *>(page);
    return our_page->processScan(control, s, len);
}
#endif  /* HAS_INSTALL_ENV */

NetworkSimpleWirelessPage::NetworkSimpleWirelessPage(QWidget *parent)
    : HorizonWizardPage(parent), control(WPACTRL_ZERO) {
    QVBoxLayout *layout;

    loadWatermark("network");
    setTitle(tr("Select Your Network"));

    statusLabel = new QLabel(tr("Scanning for networks..."));

    rescanButton = new QPushButton(tr("&Rescan Networks"));
    connect(rescanButton, &QPushButton::clicked, [=](void) { doScan(); });

    ssidListView = new QListWidget;

#ifdef HAS_INSTALL_ENV
    exchange_item.filter = "CTRL-EVENT-SCAN-RESULTS";
    exchange_item.cb = &scanResults;
    notify = nullptr;
#endif  /* HAS_INSTALL_ENV */

    passphrase = new QLineEdit(this);
    passphrase->setEchoMode(QLineEdit::Password);
    passphrase->setPlaceholderText(tr("Passphrase"));
    passphrase->hide();

    layout = new QVBoxLayout;
    layout->addWidget(statusLabel, 0, Qt::AlignCenter);
    layout->addSpacing(10);
    layout->addWidget(ssidListView, 0, Qt::AlignCenter);
    layout->addWidget(rescanButton);
    layout->addSpacing(10);
    layout->addWidget(passphrase);
    setLayout(layout);
}

NetworkSimpleWirelessPage::~NetworkSimpleWirelessPage() {
#ifdef HAS_INSTALL_ENV
    wpactrl_end(&control);
#endif  /* HAS_INSTALL_ENV */
}

void NetworkSimpleWirelessPage::scanDone(QString message) {
    rescanButton->setEnabled(true);
    statusLabel->setText(message);
}

void NetworkSimpleWirelessPage::doScan() {
#ifdef HAS_INSTALL_ENV
    ssidListView->clear();
    rescanButton->setEnabled(false);
    statusLabel->setText(tr("Scanning for networks..."));

    tain_t deadline;
    wparesponse_t response;
    std::string suppsock = "/var/run/wpa_supplicant/" +
            horizonWizard()->chosen_auto_iface;

    tain_now_g();
    if(!wpactrl_start_g(&control, suppsock.c_str(), 2000)) {
        rescanButton->setEnabled(false);
        statusLabel->setText(tr("Couldn't communicate with wireless subsystem (Code %1)").arg(errno));
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

        if(wpactrl_xchg_timeout_g(&control, &exchange)) {
            status = tr("Network scan timed out.");
        } else {
            if(wpactrl_update(&control) < 0) {
                status = tr("Issue communicating with wireless subsystem.");
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
                    status = tr("Scan successful.");
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
#endif  /* HAS_INSTALL_ENV */
}

void NetworkSimpleWirelessPage::initializePage() {
    doScan();
}

bool NetworkSimpleWirelessPage::isComplete() const {
    return (ssidListView->currentRow() != -1);
}

int NetworkSimpleWirelessPage::nextId() const {
    return HorizonWizard::Page_Network_DHCP;
}

#ifdef HAS_INSTALL_ENV
int NetworkSimpleWirelessPage::processScan(wpactrl_t *c, const char *, size_t) {
    assert(c == &control);

    size_t bufsize = 32768;
    char *buf = static_cast<char *>(malloc(bufsize));
    ssize_t res_size;

    errno = 0;
    res_size = wpactrl_query_g(&control, "SCAN_RESULTS", buf, bufsize);
    if(res_size == -1) {
        if(errno == EMSGSIZE) {
            statusLabel->setText(tr("Scan failed: Out of memory"));
            free(buf);
            return 0;
        } else {
            statusLabel->setText(tr("Scan failed (Code %1)").arg(errno));
            free(buf);
            return 0;
        }
    }

    std::string raw_nets(buf, static_cast<std::string::size_type>(res_size));
    std::istringstream net_streams(raw_nets);
    /* discard the first line - it's a header */
    net_streams.getline(buf, static_cast<std::streamsize>(bufsize));
    /* process networks */
    while(net_streams.getline(buf, static_cast<std::streamsize>(bufsize))) {
        std::string net_line(buf);
        std::string::size_type cur = 0, next = net_line.find_first_of('\t');
        assert(next != std::string::npos);
        std::string bssid(net_line.substr(cur, next));
        cur = next + 1;
        next = net_line.find_first_of('\t', cur);
        assert(next != std::string::npos);
        std::string freq(net_line.substr(cur, next));
        cur = next + 1;
        next = net_line.find_first_of('\t', cur);
        assert(next != std::string::npos);
        std::string signal(net_line.substr(cur, next));
        cur = next + 1;
        next = net_line.find_first_of('\t', cur);
        assert(next != std::string::npos);
        std::string flags(net_line.substr(cur, next));
        cur = next + 1;
        next = net_line.find_first_of('\t', cur);
        assert(next == std::string::npos);
        std::string ssid(net_line.substr(cur, next));

        QIcon icon;
        int strength = std::stoi(signal);
        if(strength < -90) {
            icon = QIcon::fromTheme("network-wireless-signal-none");
        } else if(strength < -80) {
            icon = QIcon::fromTheme("network-wireless-signal-weak");
        } else if(strength < -67) {
            icon = QIcon::fromTheme("network-wireless-signal-ok");
        } else if(strength < -50) {
            icon = QIcon::fromTheme("network-wireless-signal-good");
        } else {
            icon = QIcon::fromTheme("network-wireless-signal-excellent");
        }

        QListWidgetItem *network = new QListWidgetItem(ssidListView);
        network->setText(QString::fromStdString(ssid));
        network->setIcon(icon);
        network->setToolTip(tr("Frequency: %1 MHz\nBSSID: %2\nRSSI: %3")
                            .arg(freq.c_str()).arg(bssid.c_str())
                            .arg(signal.c_str()));
    }

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
    return true;
}
