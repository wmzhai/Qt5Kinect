#include "stdafx.h"
#include "QKinectGrabber.h"



class QKinectGrabberPrivate
{
public:
	QKinectGrabberPrivate();
	bool InitializeSensor();
	void UninitializeSensor();
	bool UpdateColor();
	bool UpdateDepth();


	IKinectSensor*				KinectSensor;		// Current Kinect	

	bool						EmitImageEnabled;
	QVector<QRgb>				ColorTable;
	QMutex						Mutex;
	bool						Running;

	//Color Frame
	IColorFrameReader*			ColorFrameReader;	// Color reader	
	std::vector<unsigned char>	ColorBuffer;
	unsigned short				ColorFrameWidth;		// = 1920;
	unsigned short				ColorFrameHeight;		// = 1080;
	const unsigned short		ColorFrameChannels;		// = 4;
	signed __int64				ColorFrameTime;			// timestamp


	//Depth Frame
	IDepthFrameReader*			DepthFrameReader;	// Depth reader	
	std::vector<unsigned short>	DepthBuffer;
	unsigned short				DepthFrameWidth;		// = 512;
	unsigned short				DepthFrameHeight;		// = 424;
	signed __int64				DepthFrameTime;			// timestamp
	unsigned short				DepthMinReliableDistance;
	unsigned short				DepthMaxDistance;

};

QKinectGrabberPrivate::QKinectGrabberPrivate():
	KinectSensor(NULL),
	ColorFrameReader(NULL),
	ColorFrameWidth(1920),
	ColorFrameHeight(1080),
	ColorFrameChannels(4),
	DepthFrameReader(NULL),
	DepthFrameWidth(512),
	DepthFrameHeight(424),
	EmitImageEnabled(true),
	Running(false)
{
	ColorBuffer.resize(ColorFrameWidth * ColorFrameHeight * ColorFrameChannels, 0);
	DepthBuffer.resize(DepthFrameWidth * DepthFrameHeight, 0);

	for (int i = 0; i < 256; ++i)
		ColorTable.push_back(qRgb(i, i, i));
}




QKinectGrabber::QKinectGrabber(QObject *parent)
	: QThread(parent), d_ptr(new QKinectGrabberPrivate)
{

}

QKinectGrabber::~QKinectGrabber()
{
	stop();
}



bool QKinectGrabberPrivate::InitializeSensor()
{
	HRESULT hr;

	hr = GetDefaultKinectSensor(&KinectSensor);
	if (FAILED(hr))
	{
		return false;
	}

	if (KinectSensor)
	{
		// Initialize the Kinect and get the color reader
		IColorFrameSource* pColorFrameSource = NULL;
		IDepthFrameSource* pDepthFrameSource = NULL;
		IInfraredFrameSource* pInfraredFrameSource = NULL;

		hr = KinectSensor->Open();

		if (SUCCEEDED(hr))
		{
			hr = KinectSensor->get_ColorFrameSource(&pColorFrameSource);
		}

		if (SUCCEEDED(hr))
		{
			hr = pColorFrameSource->OpenReader(&ColorFrameReader);
		}
		
		// DepthFrame
		if (SUCCEEDED(hr))
		{
			hr = KinectSensor->get_DepthFrameSource(&pDepthFrameSource);
		}

		if (SUCCEEDED(hr))
		{
			hr = pDepthFrameSource->OpenReader(&DepthFrameReader);
		}

		SafeRelease(pColorFrameSource);
		SafeRelease(pDepthFrameSource);
	}

	if (!KinectSensor || FAILED(hr))
	{
		std::cerr << "No ready Kinect found!" << std::endl;
		return false;
	}

	return true;
}


void QKinectGrabberPrivate::UninitializeSensor()
{

	// done with color frame reader
	SafeRelease(ColorFrameReader);

	// close the Kinect Sensor
	if (KinectSensor)
	{
		KinectSensor->Close();
	}

	SafeRelease(KinectSensor);
}




/// <summary>
/// Get color frame from kinect
/// </summary>
bool QKinectGrabberPrivate::UpdateColor()
{
	if (!ColorFrameReader)
	{
		return false;
	}

	IColorFrame* pColorFrame = NULL;

	HRESULT hr = ColorFrameReader->AcquireLatestFrame(&pColorFrame);

	if (SUCCEEDED(hr))
	{
		INT64 nTime = 0;
		IFrameDescription* pFrameDescription = NULL;
		int frameWidth = 0;
		int frameHeight = 0;
		ColorImageFormat imageFormat = ColorImageFormat_None;
		BYTE *pBuffer = NULL;

		hr = pColorFrame->get_RelativeTime(&nTime);

		if (SUCCEEDED(hr))
		{
			hr = pColorFrame->get_FrameDescription(&pFrameDescription);
		}

		if (SUCCEEDED(hr))
		{
			hr = pFrameDescription->get_Width(&frameWidth);
		}

		if (SUCCEEDED(hr))
		{
			hr = pFrameDescription->get_Height(&frameHeight);
		}

		if (SUCCEEDED(hr))
		{
			hr = pColorFrame->get_RawColorImageFormat(&imageFormat);
		}

		if (SUCCEEDED(hr))
		{
			if (imageFormat == ColorImageFormat_Bgra)
			{
				UINT bufferSize;
				hr = pColorFrame->AccessRawUnderlyingBuffer(&bufferSize, reinterpret_cast<BYTE**>(pBuffer));

				// copy data to color buffer
				if (SUCCEEDED(hr))
				{
					Mutex.lock();
					{
						std::copy(reinterpret_cast<unsigned char*>(pBuffer), pBuffer + bufferSize, ColorBuffer.begin());
						ColorFrameTime = nTime;

						if (ColorFrameWidth != frameWidth || ColorFrameHeight != frameHeight)
						{
							std::cerr << "<Warning>	Unexpected size for depth buffer" << std::endl;
							ColorFrameWidth = frameWidth;
							ColorFrameHeight = frameHeight;
						}
					}
					Mutex.unlock();
				}
			}
			else
			{
				Mutex.lock();
				{
					hr = pColorFrame->CopyConvertedFrameDataToArray(ColorBuffer.size(), reinterpret_cast<BYTE*>(ColorBuffer.data()), ColorImageFormat_Bgra);
					if (SUCCEEDED(hr))
					{
						ColorFrameTime = nTime;
						if (ColorFrameWidth != frameWidth || ColorFrameHeight != frameHeight)
						{
							std::cerr << "<Warning>	Unexpected size for depth buffer" << std::endl;
							ColorFrameWidth = frameWidth;
							ColorFrameHeight = frameHeight;
						}
					}
					else
					{
						std::cerr << "<Error>	Could not convert data from color frame to color buffer" << std::endl;
					}
				}
				Mutex.unlock();
			}
		}

		SafeRelease(pFrameDescription);
	}

	SafeRelease(pColorFrame);

	if (!SUCCEEDED(hr))
		return false;

	return true;
}


bool QKinectGrabberPrivate::UpdateDepth()
{
	if (!DepthFrameReader)
	{
		return false;
	}

	IDepthFrame* pDepthFrame = NULL;

	HRESULT hr = DepthFrameReader->AcquireLatestFrame(&pDepthFrame);

	if (SUCCEEDED(hr))
	{
		INT64 nTime = 0;
		IFrameDescription* pFrameDescription = NULL;
		int frameWidth = 0;
		int frameHeight = 0;
		USHORT nDepthMinReliableDistance = 0;
		USHORT nDepthMaxDistance = 0;
		UINT nBufferSize = 0;
		UINT16 *pBuffer = NULL;

		hr = pDepthFrame->get_RelativeTime(&nTime);

		if (SUCCEEDED(hr))
		{
			hr = pDepthFrame->get_FrameDescription(&pFrameDescription);
		}

		if (SUCCEEDED(hr))
		{
			hr = pFrameDescription->get_Width(&frameWidth);
		}

		if (SUCCEEDED(hr))
		{
			hr = pFrameDescription->get_Height(&frameHeight);
		}

		if (SUCCEEDED(hr))
		{
			hr = pDepthFrame->get_DepthMinReliableDistance(&nDepthMinReliableDistance);
		}

		if (SUCCEEDED(hr))
		{
			// In order to see the full range of depth (including the less reliable far field depth)
			// we are setting nDepthMaxDistance to the extreme potential depth threshold
			//nDepthMaxDistance = USHRT_MAX;

			// Note:  If you wish to filter by reliable depth distance, uncomment the following line.
			hr = pDepthFrame->get_DepthMaxReliableDistance(&nDepthMaxDistance);
		}

		if (SUCCEEDED(hr))
		{
			hr = pDepthFrame->AccessUnderlyingBuffer(&nBufferSize, &pBuffer);
		}


		if (SUCCEEDED(hr))
		{
			Mutex.lock();
			{
				// copy data to depth buffer
				std::copy(reinterpret_cast<unsigned short*>(pBuffer), pBuffer + nBufferSize, DepthBuffer.begin());
				DepthMinReliableDistance = nDepthMinReliableDistance;
				DepthMaxDistance = nDepthMaxDistance;
				DepthFrameTime = nTime;

				if (DepthFrameWidth != frameWidth || DepthFrameHeight != frameHeight)
				{
					std::cerr << "<Warning>	Unexpected size for depth buffer" << std::endl;
					DepthFrameWidth = frameWidth;
					DepthFrameHeight = frameHeight;
				}
			}
			Mutex.unlock();
		}

		SafeRelease(pFrameDescription);
	}

	SafeRelease(pDepthFrame);

	if (!SUCCEEDED(hr))
		return false;

	return true;
}

void QKinectGrabber::stop()
{
	Q_D(QKinectGrabber);
	d->Mutex.lock();
	{
		d->Running = false;
	}
	d->Mutex.unlock();

	wait();

	d->UninitializeSensor();
}



void QKinectGrabber::run()
{
	Q_D(QKinectGrabber);

	if (!d->InitializeSensor())
	{
		std::cerr << "<Error> Kinect not started" << std::endl;
		return;
	}

	d->Running = true;

	while (d->Running)
	{
		bool colorUpdated = d->UpdateColor();
		bool depthUpdated = d->UpdateDepth();
		

		if (colorUpdated || depthUpdated)
			emit frameUpdated();

		// If send image is enabled, emit signal with the color image
		if (colorUpdated && d->EmitImageEnabled)
		{
			d->Mutex.lock();
			{
				emit colorImage(QImage(d->ColorBuffer.data(), d->ColorFrameWidth, d->ColorFrameHeight, QImage::Format_ARGB32));
			}
			d->Mutex.unlock();
		}



		// If send image is enabled, emit signal with the depth image
		if (depthUpdated && d->EmitImageEnabled)
		{
			d->Mutex.lock();
			{
				// create depth image
				QImage depthImg = QImage(d->DepthFrameWidth, d->DepthFrameHeight, QImage::Format::Format_Indexed8);
				depthImg.setColorTable(d->ColorTable);

				std::vector<unsigned char> depthImgBuffer(d->DepthBuffer.size());

				// casting from unsigned short (2 bytes precision) to unsigned char (1 byte precision)
				std::transform(
					d->DepthBuffer.begin(),
					d->DepthBuffer.end(),
					depthImgBuffer.begin(),
					[=](const unsigned short depth) { return static_cast<unsigned char>((float)depth / (float)d->DepthMaxDistance * 255.f); });

				// set pixels to depth image
				for (int y = 0; y < depthImg.height(); y++)
					memcpy(depthImg.scanLine(y), depthImgBuffer.data() + y * depthImg.width(), depthImg.width());

				emit depthImage(depthImg);
			}
			d->Mutex.unlock();
		}

		msleep(3);
	}

}