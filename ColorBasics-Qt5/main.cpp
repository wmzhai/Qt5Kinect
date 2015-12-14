#include "stdafx.h"
#include "ColorBasics.h"
#include <QtWidgets/QApplication>
#include "QKinectGrabber.h"
#include "QImageWidget.h"
#include "QD2DWidget.h"

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);


	//QImageWidget colorWidget;
	//colorWidget.setMinimumSize(1920, 1080);
	//colorWidget.show();
	//QApplication::connect(&k, SIGNAL(colorImage(QImage)), &colorWidget, SLOT(setImage(QImage)));

	ColorBasics w;
	w.resize(1920, 1080);
	w.show();

	return a.exec();
}
