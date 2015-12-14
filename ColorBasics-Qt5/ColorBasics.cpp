#include "stdafx.h"
#include "ColorBasics.h"

ColorBasics::ColorBasics(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);

	k.setUseColorFrame(true);

	connect(&k, SIGNAL(colorBuffer(const BYTE*)), ui.d2dWidget, SLOT(setColorBuffer(const BYTE*)));
	

	this->ui.d2dWidget->Initialize();

	k.start();



	//w.Initialize();
	//w.show();
}

ColorBasics::~ColorBasics()
{

}
