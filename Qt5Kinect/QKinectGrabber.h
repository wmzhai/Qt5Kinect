#pragma once


class QKinectGrabberPrivate;
class QKinectGrabber : public QThread
{
	Q_OBJECT

public:
	QKinectGrabber(QObject *parent = 0);
	~QKinectGrabber();

private:
	Q_PROPERTY(bool useColorFrame READ useColorFrame WRITE setUseColorFrame)


public:
	bool useColorFrame() const;
	void setUseColorFrame(bool);



public slots:
	void stop();

signals:
	void colorImage(const QImage &image);
	void depthImage(const QImage &image);
	void infraredImage(const QImage &image);
	void frameUpdated();

protected:
	void run() Q_DECL_OVERRIDE;

protected:
	QScopedPointer<QKinectGrabberPrivate> d_ptr;

private:
	Q_DECLARE_PRIVATE(QKinectGrabber);
	Q_DISABLE_COPY(QKinectGrabber);
};

