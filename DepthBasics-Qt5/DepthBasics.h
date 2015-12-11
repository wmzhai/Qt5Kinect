#ifndef DEPTHBASICS_H
#define DEPTHBASICS_H

#include <QtWidgets/QWidget>
#include "ui_DepthBasics.h"

class DepthBasics : public QWidget
{
	Q_OBJECT

public:
	DepthBasics(QWidget *parent = 0);
	~DepthBasics();

private:
	Ui::DepthBasicsClass ui;
};

#endif // DEPTHBASICS_H
