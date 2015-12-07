#include "stdafx.h"
#include "QD2DWidget.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	QD2DWidget w;
	w.show();
	return a.exec();
}
