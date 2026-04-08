#include <QApplication>
#include "displaywidget.h"

int main(int argc, char* argv[])
{
    QApplication a(argc, argv);

    DisplayWidget w;
    w.resize(500, 400);
    w.show();

    return a.exec();
}