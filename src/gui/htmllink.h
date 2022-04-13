#pragma once

#include <QString>
#include <QWebElement>

class HtmlLink
{
public:
	HtmlLink(void);
	HtmlLink(QWebElement &link);
	~HtmlLink(void);
    void Make(const QString &text, const QString &url);
	QString GetText();
	QString GetUrl();
private:
	QWebElement m_link;
};
