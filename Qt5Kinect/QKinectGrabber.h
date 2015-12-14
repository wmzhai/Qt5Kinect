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
	Q_PROPERTY(bool useDepthFrame READ useDepthFrame WRITE setUseDepthFrame)
	Q_PROPERTY(bool useInfraredFrame READ useInfraredFrame WRITE setUseInfraredFrame)
	Q_PROPERTY(bool useBodyFrame READ useBodyFrame WRITE setUseBodyFrame)

public:
	bool useColorFrame() const;
	void setUseColorFrame(bool);
	bool useDepthFrame() const;
	void setUseDepthFrame(bool);
	bool useInfraredFrame() const;
	void setUseInfraredFrame(bool);
	bool useBodyFrame() const;
	void setUseBodyFrame(bool);

public slots:
	void stop();

signals:
	void colorImage(const QImage &image);
	void depthImage(const QImage &image);
	void infraredImage(const QImage &image);
	void frameUpdated();
	void colorBuffer(const BYTE* pBuf);

protected:
	void run() Q_DECL_OVERRIDE;

protected:
	QScopedPointer<QKinectGrabberPrivate> d_ptr;

private:
	Q_DECLARE_PRIVATE(QKinectGrabber);
	Q_DISABLE_COPY(QKinectGrabber);
};

