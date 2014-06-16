#include "virtualtryon.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	VirtualTryon w;
	w.show();
	return a.exec();
}
