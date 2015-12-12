#include "stdafx.h"
#include "DepthBasics.h"
#include <QtWidgets/QApplication>
#include "QKinectGrabber.h"
#include "QImageWidget.h"

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	//DepthBasics w;
	//w.show();

	QKinectGrabber k;
	k.start();

	QImageWidget colorWidget;
	colorWidget.setMinimumSize(720, 480);
	colorWidget.show();
	QApplication::connect(&k, SIGNAL(colorImage(QImage)), &colorWidget, SLOT(setImage(QImage)));

	return a.exec();
}
