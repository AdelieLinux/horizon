#ifndef NETWORKINGPAGE_HH
#define NETWORKINGPAGE_HH

#include "horizonwizardpage.hh"

#include <QButtonGroup>
#include <QRadioButton>

class NetworkingPage : public HorizonWizardPage {
public:
    NetworkingPage(QWidget *parent = 0);

    bool isComplete() const;
    int nextId() const;
private:
    QButtonGroup *radioGroup;
    QRadioButton *simple, *advanced, *skip;
};

#endif // NETWORKINGPAGE_HH
