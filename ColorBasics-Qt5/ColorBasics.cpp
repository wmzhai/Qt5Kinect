#include "stdafx.h"
#include "ColorBasics.h"

ColorBasics::ColorBasics(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);

	this->ui.d2dWidget->Initialize();


	//w.Initialize();
	//w.show();
}

ColorBasics::~ColorBasics()
{

}
