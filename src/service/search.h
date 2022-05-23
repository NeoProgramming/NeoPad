#pragma once
#include <QString>

enum ESearchMask {
	ESM_TEXT = 0x1,
	ESM_TAG = 0x2,
	ESM_ATTR = 0x4,
	ESM_TREE = 0x8
};

bool search(const QString &html, const QString &text, unsigned int sm);
