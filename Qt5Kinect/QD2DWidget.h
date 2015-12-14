#pragma once

#include <d2d1.h>
#include <d2d1helper.h>
#include <dwrite.h>
#include <wincodec.h>

class QD2DWidget : public QWidget
{
	Q_OBJECT

public:
	QD2DWidget(QWidget *parent = NULL, Qt::WindowFlags flags = NULL);
	~QD2DWidget();

	HRESULT	Initialize();
	void Uninitialize();
	
	void	ClearRenderTarget(D2D1::ColorF ClearColor);

	virtual HRESULT	render();
	void beginDraw();
	HRESULT endDraw();

	void onResize(UINT nWidth, UINT nHeight);

public slots:
	void setColorBuffer(const BYTE* pBuf);


protected:
	QPaintEngine *paintEngine() const { return 0; }
	virtual void paintEvent(QPaintEvent *e);
	virtual void resizeEvent(QResizeEvent *p_event);


private:
	ID2D1Factory*			m_pD2DFactory;
	ID2D1HwndRenderTarget*	m_pHwndRenderTarget;
	ID2D1Bitmap*            m_pBitmap;
	RGBQUAD*                m_pColorRGBX;

	// Format information
	UINT                     m_sourceHeight;
	UINT                     m_sourceWidth;
	LONG                     m_sourceStride;
};
