#include "stdafx.h"
#include "QD2DWidget.h"


QD2DWidget::QD2DWidget(QWidget *parent, Qt::WindowFlags flags ) : QWidget(parent, flags), m_pD2DFactory(0), m_sourceWidth(1920), m_sourceHeight(1080)
{
	setAttribute(Qt::WA_PaintOnScreen);
	setAttribute(Qt::WA_NoSystemBackground);

	m_sourceStride = m_sourceWidth * sizeof(RGBQUAD);

	// create heap storage for color pixel data in RGBX format
	m_pColorRGBX = new RGBQUAD[m_sourceWidth * m_sourceHeight];

	initialize();
}


QD2DWidget::~QD2DWidget()
{
	uninitialize();

	if (m_pColorRGBX)
	{
		delete[] m_pColorRGBX;
		m_pColorRGBX = NULL;
	}
}


/// <summary>
/// Initializes the default Kinect sensor
/// </summary>
/// <returns>S_OK on success, otherwise failure code</returns>
HRESULT QD2DWidget::InitializeDefaultSensor()
{
	HRESULT hr;

	hr = GetDefaultKinectSensor(&m_pKinectSensor);
	if (FAILED(hr))
	{
		return hr;
	}

	if (m_pKinectSensor)
	{
		// Initialize the Kinect and get the color reader
		IColorFrameSource* pColorFrameSource = NULL;

		hr = m_pKinectSensor->Open();

		if (SUCCEEDED(hr))
		{
			hr = m_pKinectSensor->get_ColorFrameSource(&pColorFrameSource);
		}

		if (SUCCEEDED(hr))
		{
			hr = pColorFrameSource->OpenReader(&m_pColorFrameReader);
		}

		SafeRelease(pColorFrameSource);
	}

	if (!m_pKinectSensor || FAILED(hr))
	{
		return E_FAIL;
	}

	return hr;
}

/// <summary>
/// Main processing function
/// </summary>
HRESULT QD2DWidget::Update()
{
	HRESULT hr;

	if (!m_pColorFrameReader)
	{
		return E_FAIL;
	}

	IColorFrame* pColorFrame = NULL;
	hr = m_pColorFrameReader->AcquireLatestFrame(&pColorFrame);

	if (SUCCEEDED(hr))
	{
		INT64 nTime = 0;
		IFrameDescription* pFrameDescription = NULL;
		int nWidth = 0;
		int nHeight = 0;
		ColorImageFormat imageFormat = ColorImageFormat_None;
		UINT nBufferSize = 0;
		RGBQUAD *pBuffer = NULL;

		hr = pColorFrame->get_RelativeTime(&nTime);


		if (SUCCEEDED(hr))
		{
			hr = pColorFrame->get_FrameDescription(&pFrameDescription);
		}

		if (SUCCEEDED(hr))
		{
			hr = pFrameDescription->get_Width(&nWidth);
		}

		if (SUCCEEDED(hr))
		{
			hr = pFrameDescription->get_Height(&nHeight);
		}

		if (SUCCEEDED(hr))
		{
			hr = pColorFrame->get_RawColorImageFormat(&imageFormat);
		}

		if (SUCCEEDED(hr))
		{
			if (imageFormat == ColorImageFormat_Bgra)
			{
				hr = pColorFrame->AccessRawUnderlyingBuffer(&nBufferSize, reinterpret_cast<BYTE**>(&pBuffer));
			}
			else if (m_pColorRGBX)
			{
				pBuffer = m_pColorRGBX;
				nBufferSize = m_sourceWidth * m_sourceHeight * sizeof(RGBQUAD);
				hr = pColorFrame->CopyConvertedFrameDataToArray(nBufferSize, reinterpret_cast<BYTE*>(pBuffer), ColorImageFormat_Bgra);
			}
			else
			{
				hr = E_FAIL;
			}
		}

		if (SUCCEEDED(hr))
		{
			

			// Draw the data with Direct2D
			Draw(reinterpret_cast<BYTE*>(pBuffer), m_sourceWidth * m_sourceHeight * sizeof(RGBQUAD));
		}

		SafeRelease(pFrameDescription);
	}

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

	// Begin Draw
	m_pRenderTarget->BeginDraw();
	m_pRenderTarget->SetTransform(D2D1::Matrix3x2F::Identity());
	m_pRenderTarget->Clear(D2D1::ColorF(D2D1::ColorF::Black));





	D2D1_SIZE_F rtSize = m_pRenderTarget->GetSize();





	m_pRenderTarget->DrawBitmap(m_pBitmap);


	// End Draw
	hr = m_pRenderTarget->EndDraw();
	if (hr == D2DERR_RECREATE_TARGET)
	{
		uninitialize();
		hr = initialize();
	}
	return hr;
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
	m_pDWriteFactory = 0;
	m_pRenderTarget = 0;
	m_pBitmap = 0;

	// Create D2D factory
	hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &m_pD2DFactory);

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

	// Get and initialize the default Kinect sensor
	InitializeDefaultSensor();

	return hr;
}


//-----------------------------------------------------------------------------
// Name: uninitialize()
// Desc: Releases all previously initialized objects
//-----------------------------------------------------------------------------
void QD2DWidget::uninitialize()
{
	SafeRelease(m_pRenderTarget);
	SafeRelease(m_pD2DFactory);
	SafeRelease(m_pDWriteFactory);
	SafeRelease(m_pBitmap);
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
	Update();
}


QPaintEngine * QD2DWidget::paintEngine() const { return 0; }


void QD2DWidget::paintEvent(QPaintEvent *e)
{
	Q_UNUSED(e);
	Update();
}

void QD2DWidget::resizeEvent(QResizeEvent *p_event)
{
	QSize newSize = size();
	if (p_event)
	{
		newSize = p_event->size();
		QWidget::resizeEvent(p_event);
	}
	onResize(newSize.width(), newSize.height());
}