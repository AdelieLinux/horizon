#include "netsimplewifipage.hh"

#include <QHBoxLayout>
#include <QLabel>
#include <QVBoxLayout>

#include <horizon/wirelesscontroller.hh>

NetworkSimpleWirelessPage::NetworkSimpleWirelessPage(QWidget *parent)
        : HorizonWizardPage(parent)
{
        QHBoxLayout *securityLayout;
        QVBoxLayout *layout;
        QLabel *descLabel, *securityTypeLabel, *statusLabel;

        loadWatermark("network");
        setTitle(tr("Wireless Networking Setup"));

        descLabel = new QLabel(tr(
                    "A supported Wi-Fi device has been found in this "
                    "computer.  If you connect to the Internet using "
                    "Wi-Fi, select your Access Point below.\n\n"

                    "If you don't want to use Wi-Fi, select \"Use Wired "
                    "Connection\" to continue using a wired connection."));
        descLabel->setWordWrap(true);

        statusLabel = new QLabel(tr("Scanning for networks..."));

        ssidListView = new QListView;

        ssidName = new QLineEdit(this);
        ssidName->setPlaceholderText(tr("Network Name"));
        ssidName->hide();

        securityTypeLabel = new QLabel(tr("Security type"));

        securityType = new QComboBox(this);
        securityType->addItems({tr("None"), tr("WPA2 Personal"),
                                tr("WPA2 Enterprise"), tr("WPA Personal"),
                                tr("WPA Enterprise"), tr("WEP")});
        securityType->setEnabled(false);
        securityType->hide();

        passphrase = new QLineEdit(this);
        passphrase->setEchoMode(QLineEdit::Password);
        passphrase->setPlaceholderText(tr("Passphrase"));
        passphrase->hide();

        securityLayout = new QHBoxLayout;
        securityLayout->addWidget(securityTypeLabel);
        securityLayout->addWidget(securityType);

        layout = new QVBoxLayout;
        layout->addWidget(descLabel);
        layout->addWidget(statusLabel, 0, Qt::AlignCenter);
        layout->addSpacing(10);
        layout->addWidget(ssidListView, 0, Qt::AlignCenter);
        layout->addSpacing(10);
        layout->addWidget(ssidName);
        layout->addSpacing(10);
        layout->addLayout(securityLayout);
        layout->addSpacing(10);
        layout->addWidget(passphrase);
        setLayout(layout);
}

void NetworkSimpleWirelessPage::initializePage()
{
        Horizon::WirelessController::wirelessController()->performScan();
}
