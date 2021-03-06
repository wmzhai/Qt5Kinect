#include "stdafx.h"
#include "InfraredBasics.h"
#include <QtWidgets/QApplication>
#include "QKinectGrabber.h"
#include "QImageWidget.h"


int main(int argc, char *argv[])
{
	QApplication a(argc, argv);

	QKinectGrabber k;
	k.setUseInfraredFrame(true);
	k.start();

	QImageWidget infraredWidget;
	infraredWidget.setMinimumSize(512, 424);
	infraredWidget.show();
	QApplication::connect(&k, SIGNAL(infraredImage(QImage)), &infraredWidget, SLOT(setImage(QImage)));


	return a.exec();
}
