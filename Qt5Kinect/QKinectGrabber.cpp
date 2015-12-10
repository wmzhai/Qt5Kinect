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