#include "htmllink.h"

HtmlLink::HtmlLink(void)
{
	m_link = QWebElement();
}

HtmlLink::HtmlLink(QWebElement &link)
{
	m_link = link;
}

HtmlLink::~HtmlLink(void)
{
}

QString HtmlLink::GetText()
{
	if (m_link.isNull())
		return QString();
	return m_link.toPlainText();
}

QString HtmlLink::GetUrl()
{
	if (m_link.isNull())
		return QString();
	return m_link.attribute("href");
}

void HtmlLink::Make(const QString &text, const QString &url)
{
    if (m_link.isNull())
        return;
    if(url.isEmpty()) {
        m_link.setOuterXml(text);
    }
    else {
        m_link.setPlainText(text);
        m_link.setAttribute("href", url);
    }
}
