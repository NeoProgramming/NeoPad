#include "search.h"

bool search(const QString &html, const QString &text, unsigned int sm)
{
    const QChar *p = html.constData();
    const QChar *t = text.constData();
    enum State {
        ST_TEXT,
        ST_TAG,
        ST_SQ,
        ST_DQ
    } st = ST_TEXT;

	auto search_check = [&]() {
		if (p->toLower() == t->toLower()) {
			t++;
			if (*t == 0)
				return true;
		}
		else {
			t = text.constData();
		}
		return false;
	};

    // text <tag attr1="val1"> text <tag/> text
    while(*p != 0) {
        switch(st) {
        case ST_TEXT:
			if (*p == '<') {
				st = ST_TAG;
			}
			if ((sm & ESM_TEXT) && search_check())
				return true;
            break;
        case ST_TAG:
            if(*p=='\'')
                st = ST_SQ;
            else if(*p=='"')
                st = ST_DQ;
            else if(*p=='>')
                st = ST_TEXT;
			if ((sm & ESM_TAG) && search_check())
				return true;
            break;
        case ST_SQ:
            if(*p == '\'')
                st = ST_TAG;
			if ((sm & ESM_ATTR) && search_check())
				return true;
            break;
        case ST_DQ:
            if(*p == '"')
                st = ST_TAG;
			if ((sm & ESM_ATTR) && search_check())
				return true;
            break;
        }
        p++;
    }
    return false;
}

