#include "stdafx.h"
#include "QD2DWidget.h"

QD2DWidget::QD2DWidget(QWidget *parent, Qt::WindowFlags flags)
	: QWidget(parent, flags),
	m_pD2DFactory(NULL),
	m_pWICFactory(NULL),
	m_pDWriteFactory(NULL),
	m_pHwndRenderTarget(NULL),
	m_pBrush(NULL),
	m_pTextFormat(NULL),
	m_pBitmap(NULL)

{
	setAttribute(Qt::WA_PaintOnScreen);
	setAttribute(Qt::WA_NoSystemBackground);

}

QD2DWidget::~QD2DWidget()
{
	Uninitialize();
}

HRESULT	QD2DWidget::Initialize()
{
	HRESULT hr = S_OK;

	// Get the frame size
	m_sourceWidth = 1920;
	m_sourceHeight = 1080;
	m_sourceStride = m_sourceWidth * sizeof(RGBQUAD);

	// create heap storage for color pixel data in RGBX format
	m_pColorRGBX = new RGBQUAD[m_sourceWidth * m_sourceHeight];

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

	D2D1_SIZE_U size = D2D1::SizeU(m_sourceWidth, m_sourceHeight);
	D2D1_RENDER_TARGET_PROPERTIES rtProps = D2D1::RenderTargetProperties();
	rtProps.pixelFormat = D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_IGNORE);
	rtProps.usage = D2D1_RENDER_TARGET_USAGE_GDI_COMPATIBLE;

	if (SUCCEEDED(hr))
	{
		// Create a Direct2D render target.
		hr = m_pD2DFactory->CreateHwndRenderTarget(
			D2D1::RenderTargetProperties(),
			D2D1::HwndRenderTargetProperties((HWND)this->winId(), size),
			&m_pHwndRenderTarget
			);
	}

	// Create a bitmap that we can copy image data into and then render to the target
	hr = m_pHwndRenderTarget->CreateBitmap(
		size,
		D2D1::BitmapProperties(D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_IGNORE)),
		&m_pBitmap
		);

	if (SUCCEEDED(hr))
	{
		hr = RestoreDeviceObjects();
	}

	return hr;
}


void QD2DWidget::Uninitialize()
{

	SAFE_RELEASE(m_pHwndRenderTarget);
	SAFE_RELEASE(m_pD2DFactory);
	SAFE_RELEASE(m_pWICFactory);
	SAFE_RELEASE(m_pDWriteFactory);

	if (m_pColorRGBX)
	{
		delete[] m_pColorRGBX;
		m_pColorRGBX = NULL;
	}
}


void QD2DWidget::beginDraw()
{
	m_pHwndRenderTarget->BeginDraw();
}


HRESULT QD2DWidget::endDraw()
{
	HRESULT hr = m_pHwndRenderTarget->EndDraw();
	if (hr == D2DERR_RECREATE_TARGET)
	{
		Uninitialize();
		hr = Initialize();
	}
	return hr;
}


//-----------------------------------------------------------------------------
// Name: restoreDeviceObjects()
// Desc: You are encouraged to develop applications with a single code path to 
//       respond to device loss. This code path is likely to be similar, if not 
//       identical, to the code path taken to initialize the device at startup.
//-----------------------------------------------------------------------------
HRESULT	QD2DWidget::RestoreDeviceObjects()
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
	hr = m_pHwndRenderTarget->CreateSolidColorBrush(
		D2D1::ColorF(D2D1::ColorF::White),
		&m_pBrush
		);

	return S_OK;
}

//-----------------------------------------------------------------------------
// Name: invalidateDeviceObjects()
// Desc: If the lost device can be restored, the application prepares the 
//       device by destroying all video-memory resources and any 
//       swap chains. This is typically accomplished by using the SAFE_RELEASE 
//       macro.
//-----------------------------------------------------------------------------
HRESULT	QD2DWidget::InvalidateDeviceObjects()
{
	if (!m_pD2DFactory || !m_pDWriteFactory) return E_FAIL;

	SAFE_RELEASE(m_pBrush);
	SAFE_RELEASE(m_pTextFormat);

	return S_OK;
}

void QD2DWidget::ClearRenderTarget(D2D1::ColorF ClearColor)
{
	m_pHwndRenderTarget->Clear(ClearColor);
}


//-----------------------------------------------------------------------------
// Name: render()
// Desc: Draws the scene
//-----------------------------------------------------------------------------
HRESULT	QD2DWidget::render()
{
	if (!m_pHwndRenderTarget) return E_FAIL;
	if ((m_pHwndRenderTarget->CheckWindowState() & D2D1_WINDOW_STATE_OCCLUDED)) return E_FAIL;

	HRESULT hr = S_OK;

	static const WCHAR sc_helloWorld[] = L"Hello, World!";

	beginDraw();

	m_pHwndRenderTarget->SetTransform(D2D1::Matrix3x2F::Identity());

	ClearRenderTarget(D2D1::ColorF(D2D1::ColorF::Black));

	m_pHwndRenderTarget->DrawText(
		sc_helloWorld,
		ARRAYSIZE(sc_helloWorld) - 1,
		m_pTextFormat,
		D2D1::RectF(0, 0, width() / 2, height() / 2),
		m_pBrush
		);

	endDraw();

	return S_OK;
}

void QD2DWidget::onResize(UINT nWidth, UINT nHeight)
{
	HRESULT hr = S_OK;

	if (m_pHwndRenderTarget)
	{
		// Note: This method can fail, but it's okay to ignore the
		// error here -- it will be repeated on the next call to
		// EndDraw.
		D2D1_SIZE_U size;
		size.width = nWidth;
		size.height = nHeight;

		m_pHwndRenderTarget->Resize(size);
	}
	render();
}


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


void QD2DWidget::setColorBuffer(const BYTE* pBuf)
{
	HRESULT hr = S_OK;

	// Copy the image that was passed in into the direct2d bitmap
	hr = m_pBitmap->CopyFromMemory(NULL, pBuf, m_sourceStride);

	if (FAILED(hr))
		return ;

	beginDraw();

	// Draw the bitmap stretched to the size of the window
	m_pHwndRenderTarget->DrawBitmap(m_pBitmap);

	endDraw();

}