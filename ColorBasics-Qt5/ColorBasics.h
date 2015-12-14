#pragma once
#include <QtWidgets/QWidget>
#include "ui_ColorBasics.h"
#include "QD2DWidget.h"
#include "QKinectGrabber.h"


class ColorBasics : public QWidget
{
	Q_OBJECT

public:
	ColorBasics(QWidget *parent = 0);
	~ColorBasics();

private:
	Ui::ColorBasicsClass ui;
	QKinectGrabber k;
};
