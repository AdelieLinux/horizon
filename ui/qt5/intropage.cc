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
                tr("The streamlined installation process for Adélie "
                   "Linux will only take about 10-15 minutes of your "
                   "time.  After you're done, your computer will be "
                   "running the reliable, secure, libre Adélie Linux "
                   "operating system.\n\n"

                   "When you're ready to answer a few questions, get "
                   "started by choosing Next.  If you'd like more "
                   "information about the installation procedure, "
                   "choose Help at any time.\n\n"

                   "If you are unable to use a mouse, you may press "
                   "the Tab key to cycle between the available buttons."
                   "  The currently selected button will be highlighted.  "
                   "Press the Return key to make your selection."));
#endif
    descLabel->setWordWrap(true);

    layout = new QVBoxLayout;
    layout->addWidget(descLabel);
    setLayout(layout);
}

int IntroPage::nextId() const {
    return HorizonWizard::Page_Input;
}
