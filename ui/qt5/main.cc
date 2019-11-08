#include <QApplication>
#include <QLibraryInfo>
#include <QTranslator>

#include "horizonwizard.hh"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    QString translatorFileName = QLatin1String("qt_");
    translatorFileName += QLocale::system().name();
    QTranslator *translator = new QTranslator(&app);
    if(translator->load(translatorFileName,
                        QLibraryInfo::location(
                                QLibraryInfo::TranslationsPath
                        ))) {
        app.installTranslator(translator);
    }

    HorizonWizard wizard;
    wizard.show();

    return app.exec();
}
