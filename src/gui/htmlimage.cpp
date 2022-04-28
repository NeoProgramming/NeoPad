#include "htmlimage.h"
#include <QFile>
#include <QFileInfo>

HtmlImage::HtmlImage()
{
	m_image = QWebElement();
}

HtmlImage::HtmlImage(QWebElement &image)
{
	m_image = image;
}

HtmlImage::~HtmlImage(void)
{
}

QString HtmlImage::GetPath()
{
	if (m_image.isNull())
		return QString();
	return m_image.attribute("src");
}

int HtmlImage::GetWidth()
{
	if (m_image.isNull())
		return 0;
	QString width = m_image.attribute("width");
	if (width.endsWith('%'))
		return -width.toInt();
	return width.toInt();
}

int HtmlImage::GetHeight()
{
	if (m_image.isNull())
		return 0;
	QString height = m_image.attribute("height");
	if (height.endsWith('%'))
		return -height.toInt();
	return height.toInt();
}

void HtmlImage::SetPath(const QString &fpath)
{
	m_image.setAttribute("src", fpath);
}

void HtmlImage::SetWidth(int w)
{
	if(w < 0)
		m_image.setAttribute("width", QString::asprintf("%d%%", -w));
	else if(w > 0)
		m_image.setAttribute("width", QString::asprintf("%d", w));
	else
		m_image.removeAttribute("width");
}

void HtmlImage::SetHeight(int h)
{
	if (h < 0)
		m_image.setAttribute("height", QString::asprintf("%d%%", -h));
	else if (h > 0)
		m_image.setAttribute("height", QString::asprintf("%d", h));
	else
		m_image.removeAttribute("height");
}

