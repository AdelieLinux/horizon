#ifndef WELCOMEPAGE_HH
#define WELCOMEPAGE_HH

#include "horizonwizardpage.hh"

class IntroPage : public HorizonWizardPage {
public:
    IntroPage(QWidget *parent = 0);
    int nextId() const;
};

#endif // WELCOMEPAGE_HH
