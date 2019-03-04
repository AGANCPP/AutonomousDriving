
#include "AutonomousDriving.h"
#include "mainwindow.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    //AutonomousDriving w;
    MainWindow w;
    w.show();
    return a.exec();
}
