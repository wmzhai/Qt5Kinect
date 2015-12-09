#include "stdafx.h"
#include "ColorBasics.h"
#include <QtWidgets/QApplication>
#include "QKinectGrabber.h"

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	
	QKinectGrabber k;
	k.start();

	return a.exec();
}
