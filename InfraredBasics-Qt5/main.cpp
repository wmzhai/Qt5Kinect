#include "stdafx.h"
#include "InfraredBasics.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	InfraredBasics w;
	w.show();
	return a.exec();
}
