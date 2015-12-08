#include "stdafx.h"
#include "QD2DWidget.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	QD2DWidget w;
	w.resize(1920, 1080);
	w.show();
	return a.exec();
}
