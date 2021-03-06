#include "stdafx.h"
#include "QImageWidget.h"

QImageWidget::QImageWidget(QWidget* parent) : QLabel(parent)
{
}


void QImageWidget::setImage(const QImage& image)
{
	if (!image.isNull())
	{
		//resize(image.size());
		//setPixmap(QPixmap::fromImage(image));
		setPixmap(QPixmap::fromImage(image).scaled(width(), height(), Qt::KeepAspectRatio));
	}
}


bool QImageWidget::loadFile(const QString &fileName)
{
	QImage image = QImage(fileName);
	if (image.isNull())
	{
		QMessageBox::information(this, QGuiApplication::applicationDisplayName(),
			tr("Cannot load %1.").arg(QDir::toNativeSeparators(fileName)));
		setWindowFilePath(QString());
		setPixmap(QPixmap());
		adjustSize();
		return false;
	}

	setImage(image);

	setWindowFilePath(fileName);
	return true;
}

