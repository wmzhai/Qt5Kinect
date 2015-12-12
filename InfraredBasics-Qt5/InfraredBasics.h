#ifndef INFRAREDBASICS_H
#define INFRAREDBASICS_H

#include <QtWidgets/QWidget>
#include "ui_InfraredBasics.h"

class InfraredBasics : public QWidget
{
	Q_OBJECT

public:
	InfraredBasics(QWidget *parent = 0);
	~InfraredBasics();

private:
	Ui::InfraredBasicsClass ui;
};

#endif // INFRAREDBASICS_H
