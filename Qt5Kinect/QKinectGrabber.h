#pragma once




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


class QKinectGrabber : public QThread
{
	Q_OBJECT

public:
	QKinectGrabber(QObject *parent = 0);
	~QKinectGrabber();

private:
	
};

