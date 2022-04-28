#pragma once

#include <QString>
#include <QWebElement>

class HtmlImage
{
public:
	HtmlImage();
	HtmlImage(QWebElement &image);
	~HtmlImage(void);

	QString GetPath();
	int GetWidth();
	int GetHeight();

	void SetPath(const QString &fpath);
	void SetWidth(int w);
	void SetHeight(int h);
private:
	QWebElement m_image;
};
