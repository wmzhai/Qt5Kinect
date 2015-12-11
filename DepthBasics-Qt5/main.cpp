#include "stdafx.h"
#include "DepthBasics.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	DepthBasics w;
	w.show();
	return a.exec();
}
