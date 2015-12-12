#include "stdafx.h"
#include "ColorBasics.h"
#include <QtWidgets/QApplication>
#include "QKinectGrabber.h"
#include "QImageWidget.h"

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	
	QKinectGrabber k;
	k.start();

	QImageWidget colorWidget;
	colorWidget.setMinimumSize(1920, 1080);
	colorWidget.show();
	QApplication::connect(&k, SIGNAL(colorImage(QImage)), &colorWidget, SLOT(setImage(QImage)));

	return a.exec();
}
