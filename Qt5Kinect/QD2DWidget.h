#pragma once

class QD2DWidgetPrivate;
class QD2DWidget : public QWidget
{
	Q_OBJECT

public:
	QD2DWidget(QWidget *parent = NULL, Qt::WindowFlags flags = NULL);
	~QD2DWidget();

	HRESULT	Initialize();
	void Uninitialize();
	
	virtual HRESULT	render();


	void onResize(UINT nWidth, UINT nHeight);

protected:
	QScopedPointer<QD2DWidgetPrivate> d_ptr;
	void beginDraw();
	HRESULT endDraw();

public slots:
	void setColorBuffer(const BYTE* pBuf);

protected:
	QPaintEngine *paintEngine() const { return 0; }
	virtual void paintEvent(QPaintEvent *e);
	virtual void resizeEvent(QResizeEvent *p_event);
};
