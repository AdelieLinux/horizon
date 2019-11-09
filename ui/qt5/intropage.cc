#include "intropage.hh"

#include <QLabel>
#include <QVBoxLayout>

IntroPage::IntroPage(QWidget *parent) : HorizonWizardPage(parent) {
    QLabel *descLabel;
    QVBoxLayout *layout;

    loadWatermark("intro");
    setTitle(tr("Welcome to Adélie Linux"));

#ifndef HAS_INSTALL_ENV
    descLabel = new QLabel(
                tr("<p>"
                   "Horizon will guide you through creation of a basic "
                   "<code>installfile</code> "
                   "for installing Adélie Linux on another computer."
                   "<p>"
                   "<b>IMPORTANT:</b> Not all advanced settings will "
                   "be available to you.  You may be allowed to "
                   "specify an invalid or non-bootable disk layout or "
                   "network configuration.  For best results, always "
                   "run System Installation directly on the computer "
                   "you wish to run Adélie Linux."
                   "<p>"
                   "For more information about the "
                   "<code>installfile</code> "
                   "format and syntax, see the "
                   "<a href='https://help.adelielinux.org/html/install/'>"
                   "Adélie Linux Installation Guide</a> on the "
                   "Internet."));
    descLabel->setOpenExternalLinks(true);
    descLabel->setTextFormat(Qt::RichText);
#else
    descLabel = new QLabel(
                tr("Thank you for choosing the Adélie Linux operating system.  "
                   "This installation process will only take about 10-15 minutes of your time.  "
                   "When you're done, your computer will be up and running with the reliable, secure, libre Adélie Linux.\n\n"

                   "When you're ready to answer a few questions, you can get started by choosing Next or pressing F8.  "
                   "If you'd like more information about the installation procedure, choose Help or press F1 at any time.\n\n"

                   "The installation procedure will not change anything on your computer until the final step.  "
                   "You can safely cancel at any time, with no change to your computer, by choosing Cancel or pressing F3.\n\n"

                   "If you are unable to use a mouse, you may press the Tab key to cycle between the available inputs.  "
                   "The currently selected input will be highlighted."));
#endif
    descLabel->setWordWrap(true);

    layout = new QVBoxLayout;
    layout->addWidget(descLabel);
    setLayout(layout);
}

int IntroPage::nextId() const {
    return HorizonWizard::Page_Input;
}
