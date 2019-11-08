#include "networkingpage.hh"
#include "horizonwizard.hh"

#include <cstdint>
#include <QLabel>
#include <QVBoxLayout>

NetworkingPage::NetworkingPage(QWidget *parent) : HorizonWizardPage(parent)
{
    QLabel *descLabel;
    QVBoxLayout *layout;

    loadWatermark("network");
    setTitle(tr("Networking Setup"));

    descLabel = new QLabel(tr(
        "If you have a typical network connection where your computer is "
        "directly connected to the Internet via Ethernet or Wi-Fi using a "
        "modem or router, choose Automatic.  If you need to set a static IP "
        "address, or you use a VPN or proxy server, choose Manual.\n\n"

        "If you don't want to configure networking or you don't want to use "
        "this computer on the Internet, choose Skip."));
    descLabel->setWordWrap(true);

    simple = new QRadioButton(tr("&Automatic - my computer connects to the Internet directly\n"
                                 "or via a modem/router."));
    advanced = new QRadioButton(tr("&Manual - my computer connects to an enterprise network,\n"
                                   "or I use a static IP address, VPN, or 802.1X."));
    skip = new QRadioButton(tr("&Skip - I don't want to connect to a network or the Internet."));

    radioGroup = new QButtonGroup(this);
    radioGroup->addButton(simple);
    radioGroup->addButton(advanced);
    radioGroup->addButton(skip);

    QObject::connect(radioGroup, static_cast<void (QButtonGroup:: *)(QAbstractButton *)>(&QButtonGroup::buttonClicked),
                     [=](QAbstractButton *button __attribute__((unused))) {
        emit completeChanged();
    });

    layout = new QVBoxLayout;
    layout->addWidget(descLabel);
    layout->addSpacing(50);
    layout->addWidget(simple);
    layout->addWidget(advanced);
    layout->addWidget(skip);
    setLayout(layout);
}

bool NetworkingPage::isComplete() const
{
    return (radioGroup->checkedButton() != nullptr);
}

int NetworkingPage::nextId() const
{
    if(radioGroup->checkedButton() == simple) {
        /* determine wifi */
        if(false) {
            return HorizonWizard::Page_Network_Wireless;
        } else {
            if(false) {
                return HorizonWizard::Page_Network_Iface;
            } else {
                return HorizonWizard::Page_Network_DHCP;
            }
        }
    } else if(radioGroup->checkedButton() == advanced) {
        return HorizonWizard::Page_Network_Manual;
    } else {
        return HorizonWizard::Page_DateTime;
    }
}
