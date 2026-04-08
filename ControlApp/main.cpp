#include <QApplication>
#include "controlwidget.h"
#include "flightmodelthread.h"

int main(int argc, char* argv[])
{
    QApplication a(argc, argv);

    ControlWidget w;
    w.resize(300, 400);
    w.show();

    FlightModelThread model;
    model.start();

	int result = a.exec();

	model.running = false;
	model.wait();

    return result;
}