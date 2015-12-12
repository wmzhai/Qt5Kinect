#include "stdafx.h"
#include "BodyBasics.h"
#include <QtWidgets/QApplication>
#include "QKinectGrabber.h"
#include "QImageWidget.h"

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);

	QKinectGrabber k;
	k.setUseBodyFrame(true);
	k.start();

	return a.exec();
}
