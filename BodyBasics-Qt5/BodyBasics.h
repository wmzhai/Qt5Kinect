#ifndef BODYBASICS_H
#define BODYBASICS_H

#include <QtWidgets/QWidget>
#include "ui_BodyBasics.h"

class BodyBasics : public QWidget
{
	Q_OBJECT

public:
	BodyBasics(QWidget *parent = 0);
	~BodyBasics();

private:
	Ui::BodyBasicsClass ui;
};

#endif // BODYBASICS_H
