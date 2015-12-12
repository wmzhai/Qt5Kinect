#include "stdafx.h"
#include "InfraredBasics.h"
#include <QtWidgets/QApplication>
#include "QKinectGrabber.h"
#include "QImageWidget.h"


int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	//InfraredBasics w;
	//w.show();

	QKinectGrabber k;
	k.start();

	QImageWidget depthWidget;
	depthWidget.setMinimumSize(512, 424);
	depthWidget.show();
	QApplication::connect(&k, SIGNAL(depthImage(QImage)), &depthWidget, SLOT(setImage(QImage)));


	return a.exec();
}
