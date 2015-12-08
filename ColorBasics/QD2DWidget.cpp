#include "stdafx.h"
#include "QD2DWidget.h"


QD2DWidget::QD2DWidget(QWidget *parent, Qt::WindowFlags flags ) : QWidget(parent, flags), m_pD2DFactory(0), m_sourceWidth(1920), m_sourceHeight(1080)
{
	setAttribute(Qt::WA_PaintOnScreen);
	setAttribute(Qt::WA_NoSystemBackground);

	m_sourceStride = m_sourceWidth * sizeof(RGBQUAD);

	initialize();
}


QD2DWidget::~QD2DWidget()
{
	uninitialize();
}