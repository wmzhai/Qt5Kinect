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



//-----------------------------------------------------------------------------
// Name: render()
// Desc: Draws the scene
//-----------------------------------------------------------------------------
HRESULT	QD2DWidget::render()
{
	if (!m_pRenderTarget) return E_FAIL;
	if ((m_pRenderTarget->CheckWindowState() & D2D1_WINDOW_STATE_OCCLUDED)) return E_FAIL;

	HRESULT hr = S_OK;

	static const WCHAR sc_helloWorld[] = L"Hello, World!";

	beginDraw();

	m_pRenderTarget->SetTransform(D2D1::Matrix3x2F::Identity());

	clearRenderTarget(D2D1::ColorF(D2D1::ColorF::Black));

	m_pRenderTarget->DrawText(
		sc_helloWorld,
		ARRAYSIZE(sc_helloWorld) - 1,
		m_pTextFormat,
		D2D1::RectF(0, 0, width() / 2, height() / 2),
		m_pBrush
		);

	endDraw();

	return S_OK;
}


HRESULT QD2DWidget::Draw(BYTE* pImage, unsigned long cbImage)
{
	// incorrectly sized image data passed in
	if (cbImage < ((m_sourceHeight - 1) * m_sourceStride) + (m_sourceWidth * 4))
	{
		return E_INVALIDARG;
	}

	HRESULT hr = m_pBitmap->CopyFromMemory(NULL, pImage, m_sourceStride);

	if (FAILED(hr))
		return hr;

	beginDraw();


	m_pRenderTarget->DrawBitmap(m_pBitmap);

	endDraw();

	return hr;
}


//-----------------------------------------------------------------------------
// Name: invalidateDeviceObjects()
// Desc: If the lost device can be restored, the application prepares the 
//       device by destroying all video-memory resources and any 
//       swap chains. This is typically accomplished by using the SafeRelease 
//       macro.
//-----------------------------------------------------------------------------
HRESULT	QD2DWidget::invalidateDeviceObjects()
{
	if (!m_pD2DFactory || !m_pDWriteFactory) return E_FAIL;

	SafeRelease(m_pBrush);
	SafeRelease(m_pTextFormat);

	return S_OK;
}



//-----------------------------------------------------------------------------
// Name: initialize()
// Desc: This function will only be called once during the application's 
//       initialization phase. Therefore, it can't contain any resources that 
//       need to be restored every time the Direct3D device is lost or the 
//       window is resized.
//-----------------------------------------------------------------------------
HRESULT	QD2DWidget::initialize()
{
	HRESULT hr = S_OK;

	m_pD2DFactory = 0;
	m_pWICFactory = 0;
	m_pDWriteFactory = 0;
	m_pRenderTarget = 0;
	m_pBitmap = 0;

	// Create D2D factory
	hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &m_pD2DFactory);
	if (SUCCEEDED(hr))
	{
		// Create a WIC factory
		hr = CoCreateInstance(
			CLSID_WICImagingFactory,
			NULL,
			CLSCTX_INPROC_SERVER,
			IID_IWICImagingFactory,
			reinterpret_cast<void **>(&m_pWICFactory)
			);
	}
	if (SUCCEEDED(hr))
	{
		// Create DWrite factory
		hr = DWriteCreateFactory(
			DWRITE_FACTORY_TYPE_SHARED,
			__uuidof(m_pDWriteFactory),
			reinterpret_cast<IUnknown **>(&m_pDWriteFactory)
			);
	}

	D2D1_SIZE_U size = D2D1::SizeU(width(), height());

	if (SUCCEEDED(hr))
	{
		D2D1_RENDER_TARGET_PROPERTIES rtProps = D2D1::RenderTargetProperties();
		rtProps.pixelFormat = D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_IGNORE);
		rtProps.usage = D2D1_RENDER_TARGET_USAGE_GDI_COMPATIBLE;

		// Create a Direct2D render target.
		hr = m_pD2DFactory->CreateHwndRenderTarget(
			rtProps,
			D2D1::HwndRenderTargetProperties((HWND)this->winId(), size),
			&m_pRenderTarget
			);
	}

	if (SUCCEEDED(hr))
	{
		// Create a bitmap that we can copy image data into and then render to the target
		hr = m_pRenderTarget->CreateBitmap(
			size,
			D2D1::BitmapProperties(D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_IGNORE)),
			&m_pBitmap
			);
	}

	if (SUCCEEDED(hr))
	{
		hr = restoreDeviceObjects();
	}

	return hr;
}


//-----------------------------------------------------------------------------
// Name: uninitialize()
// Desc: Releases all previously initialized objects
//-----------------------------------------------------------------------------
void QD2DWidget::uninitialize()
{
	invalidateDeviceObjects();

	SafeRelease(m_pRenderTarget);
	SafeRelease(m_pD2DFactory);
	SafeRelease(m_pWICFactory);
	SafeRelease(m_pDWriteFactory);
	SafeRelease(m_pBitmap);
}



//-----------------------------------------------------------------------------
// Name: restoreDeviceObjects()
// Desc: You are encouraged to develop applications with a single code path to 
//       respond to device loss. This code path is likely to be similar, if not 
//       identical, to the code path taken to initialize the device at startup.
//-----------------------------------------------------------------------------
HRESULT	QD2DWidget::restoreDeviceObjects()
{
	if (!m_pD2DFactory || !m_pDWriteFactory) return E_FAIL;

	static const WCHAR msc_fontName[] = L"Verdana";
	static const FLOAT msc_fontSize = 50;
	HRESULT hr;

	// Create a DirectWrite text format object.
	hr = m_pDWriteFactory->CreateTextFormat(
		msc_fontName,
		NULL,
		DWRITE_FONT_WEIGHT_NORMAL,
		DWRITE_FONT_STYLE_NORMAL,
		DWRITE_FONT_STRETCH_NORMAL,
		msc_fontSize,
		L"", //locale
		&m_pTextFormat
		);

	if (SUCCEEDED(hr))
	{
		// Center the text horizontally and vertically.
		m_pTextFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);

		m_pTextFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
	}

	// Create a black brush.
	hr = m_pRenderTarget->CreateSolidColorBrush(
		D2D1::ColorF(D2D1::ColorF::White),
		&m_pBrush
		);

	return S_OK;
}



//-----------------------------------------------------------------------------
// Name: clearRenderTarget()
// Desc: Clear the render target
//-----------------------------------------------------------------------------
void QD2DWidget::clearRenderTarget(D2D1::ColorF ClearColor)
{
	m_pRenderTarget->Clear(ClearColor);
}

//-----------------------------------------------------------------------------
// Name: beginDraw()
// Desc: Begin draw
//-----------------------------------------------------------------------------
void QD2DWidget::beginDraw()
{
	m_pRenderTarget->BeginDraw();
}

//-----------------------------------------------------------------------------
// Name: endDraw()
// Desc: End draw
//-----------------------------------------------------------------------------
HRESULT	QD2DWidget::endDraw()
{
	HRESULT hr = m_pRenderTarget->EndDraw();
	if (hr == D2DERR_RECREATE_TARGET)
	{
		uninitialize();
		hr = initialize();
	}
	return hr;
}


void QD2DWidget::onResize(UINT nWidth, UINT nHeight)
{
	HRESULT hr = S_OK;

	if (m_pRenderTarget)
	{
		// Note: This method can fail, but it's okay to ignore the
		// error here -- it will be repeated on the next call to
		// EndDraw.
		D2D1_SIZE_U size;
		size.width = nWidth;
		size.height = nHeight;

		m_pRenderTarget->Resize(size);
	}
	render();
}


QPaintEngine * QD2DWidget::paintEngine() const { return 0; }


void QD2DWidget::paintEvent(QPaintEvent *e)
{
	Q_UNUSED(e);
	render();
}

void QD2DWidget::resizeEvent(QResizeEvent *p_event)
{
	QSize newSize = size();
	if (p_event)
	{
		newSize = p_event->size();
		// if( width()==newSize.width() && height()==newSize.height() ) return;
		QWidget::resizeEvent(p_event);
	}
	onResize(newSize.width(), newSize.height());
}