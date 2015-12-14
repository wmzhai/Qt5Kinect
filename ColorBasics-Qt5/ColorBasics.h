#ifndef COLORBASICS_H
#define COLORBASICS_H

#include <QtWidgets/QWidget>
#include "ui_ColorBasics.h"
#include "QD2DWidget.h"

class ColorBasics : public QWidget
{
	Q_OBJECT

public:
	ColorBasics(QWidget *parent = 0);
	~ColorBasics();

private:
	Ui::ColorBasicsClass ui;
};

#endif // COLORBASICS_H
