#include "horizonhelpwindow.hh"

#include <QDialogButtonBox>
#include <QTextEdit>
#include <QVBoxLayout>

HorizonHelpWindow::HorizonHelpWindow(QFile *helpFile, QWidget *parent) :
    QDialog(parent), helpFile(helpFile) {
    QDialogButtonBox *buttonBox;
    QTextEdit *helpText;
    QVBoxLayout *layout;

    setFixedSize(QSize(600, 400));
    setSizeGripEnabled(false);
    setWindowTitle("Horizon Help");

    helpText = new QTextEdit(this);
    helpText->setReadOnly(true);
    helpText->setHtml(helpFile->readAll().toStdString().c_str());

    buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok, this);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);

    layout = new QVBoxLayout();
    layout->addWidget(helpText);
    layout->addWidget(buttonBox);

    setLayout(layout);
}
