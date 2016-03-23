#include <QApplication>
#include <QTranslator>
#include <QLabel>
#include "mainwindow.h"

int main(int argc,char** argv){
    QApplication app(argc,argv);
    QTranslator translator;
    if (translator.load(QLocale::system(), "cppversion", "_", ":/translations")) {
        app.installTranslator(&translator);
    }

    MainWindow w;
    w.show();

    return app.exec();
}
