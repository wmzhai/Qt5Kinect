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

	//-----------------------------------------------------------------------------
	// Name: initialize()
	// Desc: This function will only be called once during the application's 
	//       initialization phase. Therefore, it can't contain any resources that 
	//       need to be restored every time the Direct3D device is lost or the 
	//       window is resized.
	//-----------------------------------------------------------------------------
	HRESULT	Initialize();
	void Uninitialize();
	
	HRESULT	InvalidateDeviceObjects();
	HRESULT	RestoreDeviceObjects();
	void	ClearRenderTarget(D2D1::ColorF ClearColor);

	virtual HRESULT	render();
	void beginDraw();
	HRESULT endDraw();

	void onResize(UINT nWidth, UINT nHeight);

protected:
	QPaintEngine *paintEngine() const { return 0; }

	virtual void paintEvent(QPaintEvent *e);
	virtual void resizeEvent(QResizeEvent *p_event);

private:
	ID2D1Factory*			m_pD2DFactory;
	IWICImagingFactory*		m_pWICFactory;
	IDWriteFactory*			m_pDWriteFactory;
	ID2D1HwndRenderTarget*	m_pHwndRenderTarget;
	ID2D1SolidColorBrush*	m_pBrush;
	IDWriteTextFormat*		m_pTextFormat;
};
