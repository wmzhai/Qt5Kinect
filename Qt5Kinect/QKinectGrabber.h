#pragma once


class QKinectGrabberPrivate;
class QKinectGrabber : public QThread
{
	Q_OBJECT

public:
	QKinectGrabber(QObject *parent = 0);
	~QKinectGrabber();


protected:
	QScopedPointer<QKinectGrabberPrivate> d_ptr;

private:
	Q_DECLARE_PRIVATE(QKinectGrabber);
	Q_DISABLE_COPY(QKinectGrabber);
};

