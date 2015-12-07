
#include <QtWidgets/QApplication>
#include "QD2DWidget.h"

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	QD2DWidget w;
	w.show();
	return a.exec();
}
