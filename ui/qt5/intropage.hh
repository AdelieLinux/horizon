#ifndef WELCOMEPAGE_HH
#define WELCOMEPAGE_HH

#ifdef HAS_INSTALL_ENV
#include <QPushButton>
#endif
#include "horizonwizardpage.hh"

class IntroPage : public HorizonWizardPage {
public:
    IntroPage(QWidget *parent = nullptr);
    int nextId() const;
#ifdef HAS_INSTALL_ENV
private:
    QPushButton *toolButton;
#endif
};

#endif // WELCOMEPAGE_HH
