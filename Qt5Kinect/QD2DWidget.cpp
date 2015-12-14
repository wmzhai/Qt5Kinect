#include "stdafx.h"
#include "QD2DWidget.h"

QD2DWidget::QD2DWidget(QWidget *parent, Qt::WindowFlags flags)
	: QWidget(parent, flags),
	m_pD2DFactory(NULL),
	m_pWICFactory(NULL),
	m_pDWriteFactory(NULL),
	m_pHwndRenderTarget(NULL),
	m_pBrush(NULL),
	m_pTextFormat(NULL)

{
	setAttribute(Qt::WA_PaintOnScreen);
	setAttribute(Qt::WA_NoSystemBackground);

}

QD2DWidget::~QD2DWidget()
{

}

HRESULT	QD2DWidget::Initialize()
{
	HRESULT hr = S_OK;

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

	if (SUCCEEDED(hr))
	{
		D2D1_SIZE_U size = D2D1::SizeU(width(), height());

		// Create a Direct2D render target.
		hr = m_pD2DFactory->CreateHwndRenderTarget(
			D2D1::RenderTargetProperties(),
			D2D1::HwndRenderTargetProperties((HWND)this->winId(), size),
			&m_pHwndRenderTarget
			);
	}



	return hr;
}


void QD2DWidget::Uninitialize()
{

	SAFE_RELEASE(m_pHwndRenderTarget);
	SAFE_RELEASE(m_pD2DFactory);
	SAFE_RELEASE(m_pWICFactory);
	SAFE_RELEASE(m_pDWriteFactory);
}