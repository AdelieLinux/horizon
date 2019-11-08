#ifndef HORIZONHELPWINDOW_HH
#define HORIZONHELPWINDOW_HH

#include <QDialog>
#include <QFile>

class HorizonHelpWindow : public QDialog {
    Q_OBJECT
public:
    explicit HorizonHelpWindow(QFile *helpFile, QWidget *parent = 0);
private:
    QFile *helpFile;
};

#endif // HORIZONHELPWINDOW_HH
