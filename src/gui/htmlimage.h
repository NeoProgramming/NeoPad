#pragma once

#include <QString>
#include <QWebElement>

class HtmlImage
{
public:
	HtmlImage();
	HtmlImage(QWebElement &image);
	~HtmlImage(void);

	QString GetSrc();
	int GetWidth();
	int GetHeight();
	bool IsEmbedded();

	void SetSrc(const QString &fpath);
	void SetWidth(int w);
	void SetHeight(int h);

	static QString ConvertToEmbedded(const QString &fpath);
	static bool ConvertToFile(const QString& data, const QString &fpath);
	static QString GetImageExt(const QString& code);
private:
	QWebElement m_image;
};
