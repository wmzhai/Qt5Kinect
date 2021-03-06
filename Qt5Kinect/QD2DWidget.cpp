#include "stdafx.h"
#include "QD2DWidget.h"

class QD2DWidgetPrivate
{
public:
	ID2D1Factory*			D2DFactory;
	ID2D1HwndRenderTarget*	D2DRenderTarget;
	ID2D1Bitmap*            D2DBitmap;
	RGBQUAD*                ColorRGBX;
	UINT                    SourceHeight;
	UINT                    SourceWidth;
	LONG                    SourceStride;
};

QD2DWidget::QD2DWidget(QWidget *parent, Qt::WindowFlags flags)
	: QWidget(parent, flags), d_ptr(new QD2DWidgetPrivate)

{
	setAttribute(Qt::WA_PaintOnScreen);
	setAttribute(Qt::WA_NoSystemBackground);
	d_ptr->D2DFactory = NULL;
	d_ptr->D2DRenderTarget = NULL;
	d_ptr->D2DBitmap = NULL;
	d_ptr->ColorRGBX = NULL;
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
	d_ptr->ColorRGBX = new RGBQUAD[d_ptr->SourceWidth * d_ptr->SourceHeight];

	// Create D2D factory
	hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &(d_ptr->D2DFactory));

	D2D1_SIZE_U size = D2D1::SizeU(d_ptr->SourceWidth, d_ptr->SourceHeight);
	D2D1_RENDER_TARGET_PROPERTIES rtProps = D2D1::RenderTargetProperties();
	rtProps.pixelFormat = D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_IGNORE);
	rtProps.usage = D2D1_RENDER_TARGET_USAGE_GDI_COMPATIBLE;

	if (SUCCEEDED(hr))
	{
		// Create a Direct2D render target.
		hr = d_ptr->D2DFactory->CreateHwndRenderTarget(
			rtProps,
			D2D1::HwndRenderTargetProperties((HWND)this->winId(), size),
			&(d_ptr->D2DRenderTarget)
			);
	}

	// Create a bitmap that we can copy image data into and then render to the target
	hr = d_ptr->D2DRenderTarget->CreateBitmap(
		size,
		D2D1::BitmapProperties(D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_IGNORE)),
		&(d_ptr->D2DBitmap)
		);

	D2D1_SIZE_F renderTargetSize = d_ptr->D2DRenderTarget->GetSize();

	return hr;
}


void QD2DWidget::Uninitialize()
{

	SAFE_RELEASE(d_ptr->D2DRenderTarget);
	SAFE_RELEASE(d_ptr->D2DFactory);

	if (d_ptr->ColorRGBX)
	{
		delete[] d_ptr->ColorRGBX;
		d_ptr->ColorRGBX = NULL;
	}
}


void QD2DWidget::beginDraw()
{
	d_ptr->D2DRenderTarget->BeginDraw();
}


HRESULT QD2DWidget::endDraw()
{
	HRESULT hr = d_ptr->D2DRenderTarget->EndDraw();
	if (hr == D2DERR_RECREATE_TARGET)
	{
		Uninitialize();
		hr = Initialize();
	}
	return hr;
}


//-----------------------------------------------------------------------------
// Name: render()
// Desc: Draws the scene
//-----------------------------------------------------------------------------
HRESULT	QD2DWidget::render()
{
	if (!d_ptr->D2DRenderTarget) return E_FAIL;
	if ((d_ptr->D2DRenderTarget->CheckWindowState() & D2D1_WINDOW_STATE_OCCLUDED)) return E_FAIL;

	HRESULT hr = S_OK;

	D2D1_SIZE_F renderTargetSize = d_ptr->D2DRenderTarget->GetSize();

	beginDraw();
	d_ptr->D2DRenderTarget->SetTransform(D2D1::Matrix3x2F::Identity());

	D2D1_RECT_F destRect = D2D1::RectF(0,0,1920,1080);
	D2D1_RECT_F sourceRect = D2D1::RectF(0, 0, 1920, 1080);

	// Draw the bitmap stretched to the size of the window
	d_ptr->D2DRenderTarget->DrawBitmap(d_ptr->D2DBitmap, &destRect, 1, D2D1_BITMAP_INTERPOLATION_MODE_NEAREST_NEIGHBOR,&sourceRect);

	endDraw();

	return S_OK;
}

void QD2DWidget::onResize(UINT nWidth, UINT nHeight)
{
	HRESULT hr = S_OK;

	if (d_ptr->D2DRenderTarget)
	{
		// Note: This method can fail, but it's okay to ignore the
		// error here -- it will be repeated on the next call to
		// EndDraw.
		D2D1_SIZE_U size;
		size.width = nWidth;
		size.height = nHeight;

		d_ptr->D2DRenderTarget->Resize(size);
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
	HRESULT hr = d_ptr->D2DBitmap->CopyFromMemory(NULL, pBuf, d_ptr->SourceStride);
	if (FAILED(hr))
		return ;

	render();
}