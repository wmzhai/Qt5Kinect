#pragma once

#include <QThread>

class QKinectThread : public QThread
{
	Q_OBJECT

public:
	QKinectThread(QObject* parent = NULL);
	virtual ~QKinectThread();

protected:

	bool m_running;

	HANDLE m_color_next_frame_event;
	HANDLE m_depth_next_frame_event;

	HANDLE m_color_stream_handle;
	HANDLE m_depth_stream_handle;
    
};

