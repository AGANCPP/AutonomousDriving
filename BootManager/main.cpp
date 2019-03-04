#include "BootManager.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    BootManager w;
    w.show();
    return a.exec();
}
