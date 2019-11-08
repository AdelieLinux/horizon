#ifndef NETWORKSIMPLEWIRELESSPAGE_HH
#define NETWORKSIMPLEWIRELESSPAGE_HH

#include "horizonwizardpage.hh"

#include <QComboBox>
#include <QLineEdit>
#include <QListView>

class NetworkSimpleWirelessPage : public HorizonWizardPage
{
public:
        NetworkSimpleWirelessPage(QWidget *parent = 0);

        void initializePage();
private:
        QComboBox *securityType;
        QLineEdit *ssidName, *passphrase;
        QListView *ssidListView;
};

#endif // NETWORKSIMPLEWIRELESSPAGE_HH
