#ifndef HORIZONWIZARDPAGE_HH
#define HORIZONWIZARDPAGE_HH

#include <QWizardPage>
#include <string>

#include "horizonwizard.hh"

class HorizonWizardPage : public QWizardPage {
public:
    HorizonWizardPage(QWidget *parent = 0) : QWizardPage(parent) {}

    void loadWatermark(std::string page);
    HorizonWizard *horizonWizard() const;
};

#endif // HORIZONWIZARDPAGE_HH
