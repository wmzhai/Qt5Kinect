#include "stdafx.h"
#include "QKinectGrabber.h"


// InfraredSourceValueMaximum is the highest value that can be returned in the InfraredFrame.
// It is cast to a float for readability in the visualization code.
#define InfraredSourceValueMaximum static_cast<float>(USHRT_MAX)

// The InfraredOutputValueMinimum value is used to set the lower limit, post processing, of the
// infrared data that we will render.
// Increasing or decreasing this value sets a brightness "wall" either closer or further away.
#define InfraredOutputValueMinimum 0.01f 

// The InfraredOutputValueMaximum value is the upper limit, post processing, of the
// infrared data that we will render.
#define InfraredOutputValueMaximum 1.0f

// The InfraredSceneValueAverage value specifies the average infrared value of the scene.
// This value was selected by analyzing the average pixel intensity for a given scene.
// Depending on the visualization requirements for a given application, this value can be
// hard coded, as was done here, or calculated by averaging the intensity for each pixel prior
// to rendering.
#define InfraredSceneValueAverage 0.08f

/// The InfraredSceneStandardDeviations value specifies the number of standard deviations
/// to apply to InfraredSceneValueAverage. This value was selected by analyzing data
/// from a given scene.
/// Depending on the visualization requirements for a given application, this value can be
/// hard coded, as was done here, or calculated at runtime.
#define InfraredSceneStandardDeviations 3.0f



class QKinectGrabberPrivate
{
public:
	QKinectGrabberPrivate();
	bool InitializeSensor();
	void UninitializeSensor();
	bool UpdateColor();
	bool UpdateDepth();
	bool UpdateInfrared();


	IKinectSensor*				KinectSensor;		// Current Kinect	

	QVector<QRgb>				ColorTable;
	QMutex						Mutex;
	bool						Running;

	//Color Frame
	bool						UseColorFrame;
	IColorFrameReader*			ColorFrameReader;	// Color reader	
	std::vector<unsigned char>	ColorBuffer;
	unsigned short				ColorFrameWidth;		// = 1920;
	unsigned short				ColorFrameHeight;		// = 1080;
	const unsigned short		ColorFrameChannels;		// = 4;
	signed __int64				ColorFrameTime;			// timestamp


	//Depth Frame
	bool						UseDepthFrame;
	IDepthFrameReader*			DepthFrameReader;	// Depth reader	
	std::vector<unsigned short>	DepthBuffer;
	unsigned short				DepthFrameWidth;		// = 512;
	unsigned short				DepthFrameHeight;		// = 424;
	signed __int64				DepthFrameTime;			// timestamp
	unsigned short				DepthMinReliableDistance;
	unsigned short				DepthMaxDistance;


	//Infrared Frame
	bool						UseInfraredFrame;
	IInfraredFrameReader*		InfraredFrameReader;	// Infrared reader
	unsigned short				InfraredFrameWidth;		// = 512;
	unsigned short				InfraredFrameHeight;	// = 424;
	signed __int64				InfraredFrameTime;		// timestamp
	std::vector<unsigned short> InfraredBuffer;

	//Body Frame
	bool						UseBodyFrame;
	IBodyFrameReader*			BodyFrameReader;
	ICoordinateMapper*			CoordinateMapper;

};

QKinectGrabberPrivate::QKinectGrabberPrivate():
	Running(false),
	KinectSensor(NULL),
	UseColorFrame(false),
	ColorFrameReader(NULL),
	ColorFrameWidth(1920),
	ColorFrameHeight(1080),
	ColorFrameChannels(4),
	UseDepthFrame(false),
	DepthFrameReader(NULL),
	DepthFrameWidth(512),
	DepthFrameHeight(424),
	UseInfraredFrame(false),
	InfraredFrameReader(NULL),
	InfraredFrameWidth(512),
	InfraredFrameHeight(424),
	UseBodyFrame(false),
	BodyFrameReader(NULL),
	CoordinateMapper(NULL)	
{
	ColorBuffer.resize(ColorFrameWidth * ColorFrameHeight * ColorFrameChannels, 0);
	DepthBuffer.resize(DepthFrameWidth * DepthFrameHeight, 0);
	InfraredBuffer.resize(DepthFrameWidth * DepthFrameHeight, 0);

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

bool QKinectGrabber::useColorFrame() const
{
	return d_ptr->UseColorFrame;
}

void QKinectGrabber::setUseColorFrame(bool use)
{
	d_ptr->UseColorFrame = use;
}

bool QKinectGrabber::useDepthFrame() const
{
	return d_ptr->UseDepthFrame;
}

void QKinectGrabber::setUseDepthFrame(bool use)
{
	d_ptr->UseDepthFrame = use;
}

bool QKinectGrabber::useInfraredFrame() const
{
	return d_ptr->UseInfraredFrame;
}

void QKinectGrabber::setUseInfraredFrame(bool use)
{
	d_ptr->UseInfraredFrame = use;
}


bool QKinectGrabber::useBodyFrame() const
{
	return d_ptr->UseBodyFrame;
}

void QKinectGrabber::setUseBodyFrame(bool use)
{
	d_ptr->UseBodyFrame = use;
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
		hr = KinectSensor->Open();

		if (UseColorFrame){
			IColorFrameSource* pColorFrameSource = NULL;
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
		
		// DepthFrame
		if (UseDepthFrame){
			IDepthFrameSource* pDepthFrameSource = NULL;
			if (SUCCEEDED(hr))
			{
				hr = KinectSensor->get_DepthFrameSource(&pDepthFrameSource);
			}

			if (SUCCEEDED(hr))
			{
				hr = pDepthFrameSource->OpenReader(&DepthFrameReader);
			}

			SafeRelease(pDepthFrameSource);
		}

		// InfraredFrame
		if (UseInfraredFrame){
			IInfraredFrameSource* pInfraredFrameSource = NULL;
			if (SUCCEEDED(hr))
			{
				hr = KinectSensor->get_InfraredFrameSource(&pInfraredFrameSource);
			}

			if (SUCCEEDED(hr))
			{
				hr = pInfraredFrameSource->OpenReader(&InfraredFrameReader);
			}

			SafeRelease(pInfraredFrameSource);
		}

		// Body Frame
		if (UseBodyFrame){			
			IBodyFrameSource* pBodyFrameSource = NULL;

			if (SUCCEEDED(hr))
			{
				hr = KinectSensor->get_CoordinateMapper(&CoordinateMapper);
			}

			if (SUCCEEDED(hr))
			{
				hr = KinectSensor->get_BodyFrameSource(&pBodyFrameSource);
			}

			if (SUCCEEDED(hr))
			{
				hr = pBodyFrameSource->OpenReader(&BodyFrameReader);
			}

			SafeRelease(pBodyFrameSource);
		}
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
	if (!ColorFrameReader || !UseColorFrame)
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
	if (!DepthFrameReader || !UseDepthFrame)
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



bool QKinectGrabberPrivate::UpdateInfrared()
{
	if (!InfraredFrameReader || !UseInfraredFrame)
	{
		return false;
	}

	IInfraredFrame* pInfraredFrame = NULL;

	HRESULT hr = InfraredFrameReader->AcquireLatestFrame(&pInfraredFrame);

	if (SUCCEEDED(hr))
	{
		INT64 nTime = 0;
		IFrameDescription* pFrameDescription = NULL;
		int frameWidth = 0;
		int frameHeight = 0;
		UINT nBufferSize = 0;
		UINT16 *pBuffer = NULL;

		hr = pInfraredFrame->get_RelativeTime(&nTime);

		if (SUCCEEDED(hr))
		{
			hr = pInfraredFrame->get_FrameDescription(&pFrameDescription);
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
			hr = pInfraredFrame->AccessUnderlyingBuffer(&nBufferSize, &pBuffer);
		}

		if (SUCCEEDED(hr))
		{
			std::copy(reinterpret_cast<unsigned short*>(pBuffer), pBuffer + nBufferSize, InfraredBuffer.begin());
			InfraredFrameTime = nTime;

			if (InfraredFrameWidth != frameWidth || InfraredFrameHeight != frameHeight)
			{
				std::cerr << "<Warning>	Unexpected size for depth buffer" << std::endl;
				InfraredFrameWidth = frameWidth;
				InfraredFrameHeight = frameHeight;
			}
		}

		SafeRelease(pFrameDescription);
	}

	SafeRelease(pInfraredFrame);

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
		bool infraredUpdated = d->UpdateInfrared();

		if (colorUpdated || depthUpdated || infraredUpdated)
			emit frameUpdated();

		// If send image is enabled, emit signal with the color image
		if (d->UseColorFrame && colorUpdated)
		{
			d->Mutex.lock();
			{
				emit colorImage(QImage(d->ColorBuffer.data(), d->ColorFrameWidth, d->ColorFrameHeight, QImage::Format_ARGB32));
			}
			d->Mutex.unlock();
		}

		// If send image is enabled, emit signal with the depth image
		if (d->UseDepthFrame && depthUpdated)
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

		// If send image is enabled, emit signal with the depth image
		if (d->UseInfraredFrame && infraredUpdated)
		{
			d->Mutex.lock();
			{
				// create depth image
				QImage infraredImg = QImage(d->InfraredFrameWidth, d->InfraredFrameHeight, QImage::Format::Format_Indexed8);
				infraredImg.setColorTable(d->ColorTable);

				std::vector<unsigned char> infraredImgBuffer(d->InfraredBuffer.size());

				// casting from unsigned short (2 bytes precision) to unsigned char (1 byte precision)
				std::transform(
					d->InfraredBuffer.begin(),
					d->InfraredBuffer.end(),
					infraredImgBuffer.begin(),
					[=](const unsigned short infra) 
					{ 
						// normalize the incoming infrared data (ushort) to a float ranging from 
						// [InfraredOutputValueMinimum, InfraredOutputValueMaximum] by
						// 1. dividing the incoming value by the source maximum value
						float intensityRatio = static_cast<float>(infra) / InfraredSourceValueMaximum;

						// 2. dividing by the (average scene value * standard deviations)
						intensityRatio /= InfraredSceneValueAverage * InfraredSceneStandardDeviations;

						// 3. limiting the value to InfraredOutputValueMaximum
						intensityRatio = min(InfraredOutputValueMaximum, intensityRatio);

						// 4. limiting the lower value InfraredOutputValueMinimym
						intensityRatio = max(InfraredOutputValueMinimum, intensityRatio);

						// 5. converting the normalized value to a byte and using the result
						// as the RGB components required by the image
						byte intensity = static_cast<byte>(intensityRatio * 255.0f);

						return static_cast<unsigned char>(intensity); 
				
					}
				);

				// set pixels to depth image
				for (int y = 0; y < infraredImg.height(); y++)
					memcpy(infraredImg.scanLine(y), infraredImgBuffer.data() + y * infraredImg.width(), infraredImg.width());

				emit infraredImage(infraredImg);
			}
			d->Mutex.unlock();
		}
		msleep(3);
	}

}