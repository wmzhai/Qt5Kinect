#include "stdafx.h"
#include "BodyBasics.h"
#include <QtWidgets/QApplication>
#include "QKinectGrabber.h"
#include "QImageWidget.h"

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);

	QKinectGrabber k;
	k.setUseDepthFrame(true);
	k.start();

	QImageWidget depthWidget;
	depthWidget.setMinimumSize(512, 424);
	depthWidget.show();
	QApplication::connect(&k, SIGNAL(depthImage(QImage)), &depthWidget, SLOT(setImage(QImage)));

	return a.exec();
}
