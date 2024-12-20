#pragma once
#include <QString>
#include <QWebElement>

// html helpers
class Html {
public:
    static QString GetClass(QWebElement &el);
    static void SetClass(QWebElement &el, const QString &cls);
};



