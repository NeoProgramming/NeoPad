#pragma once

#include <QString>
#include <QWebElement>

class HtmlImage
{
public:
	HtmlImage(QWebElement &image);
	~HtmlImage(void);
	bool Init(QWebElement &image);
	bool Apply(QWebElement &image);
	QString MakeHtml();
public:
	QString m_fpath;
	QString m_width;
	QString m_height;
	QString m_src;
	QString m_alt;
	QString m_title;
private:
	QWebElement m_image;
};
