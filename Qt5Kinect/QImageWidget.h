#pragma once

class QImageWidget : public QLabel
{
	Q_OBJECT

public:
	QImageWidget(QWidget* parent = nullptr);

public slots:
	bool loadFile(const QString &);
	void setImage(const QImage& image);
};
