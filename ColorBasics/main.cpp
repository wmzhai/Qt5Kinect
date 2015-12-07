#include "colorbasics.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	ColorBasics w;
	w.show();
	return a.exec();
}
