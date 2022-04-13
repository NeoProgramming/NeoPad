#include "htmlimage.h"

HtmlImage::HtmlImage(QWebElement &image)
{
	m_image = image;
	m_width = "100%";
	m_height = "100%";
	m_src = "";
	m_alt = "";
}

HtmlImage::~HtmlImage(void)
{
}

bool HtmlImage::Init(QWebElement &image)
{
	// read data from tag
	if(image.isNull())
		return 0;

	m_fpath       = image.attribute("src");
	m_width       = image.attribute("width");
	m_height      = image.attribute("height");

	m_alt   = image.attribute("alt");
	m_title = image.attribute("title");

	return 1;
}

bool HtmlImage::Apply(QWebElement &image)
{
	// write data to tag
	if(image.isNull())
		return 0;

	image.setAttribute("src", m_fpath);
	image.setAttribute("width", m_width);
	image.setAttribute("height", m_height);

	image.setAttribute("alt", m_alt);
	image.setAttribute("title", m_title);

	return 1;
}
