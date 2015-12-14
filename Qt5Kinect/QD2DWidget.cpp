#include "stdafx.h"
#include "QD2DWidget.h"

class QD2DWidgetPrivate
{
public:
	UINT                     SourceHeight;
	UINT                     SourceWidth;
	LONG                     SourceStride;
};

QD2DWidget::QD2DWidget(QWidget *parent, Qt::WindowFlags flags)
	: QWidget(parent, flags), d_ptr(new QD2DWidgetPrivate),
	m_pD2DFactory(NULL),
	m_pHwndRenderTarget(NULL),
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
	d_ptr->SourceWidth = 1920;
	d_ptr->SourceHeight = 1080;
	d_ptr->SourceStride = d_ptr->SourceWidth * sizeof(RGBQUAD);

	
	// create heap storage for color pixel data in RGBX format
	m_pColorRGBX = new RGBQUAD[d_ptr->SourceWidth * d_ptr->SourceHeight];

	// Create D2D factory
	hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &m_pD2DFactory);

	D2D1_SIZE_U size = D2D1::SizeU(d_ptr->SourceWidth, d_ptr->SourceHeight);
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

	return hr;
}


void QD2DWidget::Uninitialize()
{

	SAFE_RELEASE(m_pHwndRenderTarget);
	SAFE_RELEASE(m_pD2DFactory);

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

	beginDraw();

	// Draw the bitmap stretched to the size of the window
	m_pHwndRenderTarget->DrawBitmap(m_pBitmap);

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
	HRESULT hr = m_pBitmap->CopyFromMemory(NULL, pBuf, d_ptr->SourceStride);
	if (FAILED(hr))
		return ;

	render();
}