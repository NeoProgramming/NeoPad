#include "html.h"

QString Html::GetClass(QWebElement &el)
{
    return el.attribute("class");
}

void Html::SetClass(QWebElement &el, const QString &cls)
{
    if (cls.isEmpty())
        el.removeAttribute("class");
    else
        el.setAttribute("class", cls);
}

