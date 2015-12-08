#pragma once

class QD2DWidget : public QWidget
{
	Q_OBJECT

public:

	QD2DWidget(QWidget *parent = 0, Qt::WindowFlags flags = 0);

	virtual ~QD2DWidget();

	
	HRESULT Draw(BYTE* pImage, unsigned long cbImage);
	

	HRESULT	initialize();
	void	uninitialize();

	
	void onResize(UINT nWidth, UINT nHeight);

	HRESULT Update();

protected:
	QPaintEngine *paintEngine() const;
	virtual void paintEvent(QPaintEvent *e);
	virtual void resizeEvent(QResizeEvent *p_event);
	HRESULT                 InitializeDefaultSensor();
	
	ID2D1Factory*			m_pD2DFactory;
	
	IDWriteFactory*			m_pDWriteFactory;
	ID2D1HwndRenderTarget*	m_pRenderTarget;
	
	
	ID2D1Bitmap*             m_pBitmap;



	// Format information
	UINT                     m_sourceHeight;
	UINT                     m_sourceWidth;
	LONG                     m_sourceStride;


	// Current Kinect
	IKinectSensor*          m_pKinectSensor;

	// Color reader
	IColorFrameReader*      m_pColorFrameReader;

	RGBQUAD*                m_pColorRGBX;
};