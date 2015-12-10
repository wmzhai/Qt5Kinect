#include "stdafx.h"
#include "QKinectGrabber.h"




struct KinectFrameBuffer
{
	std::vector<unsigned char> color; // ColorFrame Data Buffer

	KinectFrameBuffer() :ColorWidth(1920), ColorHeight(1080), ColorChannels(4)
	{
		reset();
	}

	void reset()
	{
		color.resize(ColorWidth * ColorHeight * ColorChannels, 0);
	}

	void clear()
	{
		color.clear();
	}

	unsigned short ColorWidth;
	unsigned short ColorHeight;
	unsigned short ColorChannels;
};

class QKinectGrabberPrivate
{
public:
	QKinectGrabberPrivate();
	bool InitializeSensor();
	void UninitializeSensor();
	bool UpdateColor();


	IKinectSensor*				KinectSensor;		// Current Kinect	

	bool						EmitImageEnabled;
	QVector<QRgb>				ColorTable;
	QMutex						Mutex;
	bool						Running;

	//Color Frame Related
	IColorFrameReader*			ColorFrameReader;	// Color reader	
	std::vector<unsigned char>	ColorBuffer;
	unsigned short				ColorFrameWidth;		// = 1920;
	unsigned short				ColorFrameHeight;		// = 1080;
	const unsigned short		ColorFrameChannels;		// = 4;
	signed __int64				ColorFrameTime;			// timestamp

};

QKinectGrabberPrivate::QKinectGrabberPrivate():
	KinectSensor(NULL),
	ColorFrameReader(NULL),
	ColorFrameWidth(1920),
	ColorFrameHeight(1080),
	ColorFrameChannels(4),
	ColorBuffer(ColorFrameWidth * ColorFrameHeight *ColorFrameChannels),
	EmitImageEnabled(true),
	Running(false)
{
	for (int i = 0; i < 256; ++i)
		ColorTable.push_back(qRgb(i, i, i));
}




QKinectGrabber::QKinectGrabber(QObject *parent)
	: QThread(parent)
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

		SafeRelease(pColorFrameSource);
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

		if (colorUpdated)
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

		msleep(3);
	}

}