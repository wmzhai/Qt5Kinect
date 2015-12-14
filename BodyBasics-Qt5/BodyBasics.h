#pragma once
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
