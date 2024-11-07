#include "awindow.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    AWindow w;
    w.show();
    return a.exec();
}
