#include "vmbsrv.h"
#include <QClipboard>
#include <QApplication>
#include <QMimeData>

// tools

void CopyLink(const char *uid)
{
	QClipboard *clipboard = QApplication::clipboard();
	QMimeData *mimeData = new QMimeData;
	mimeData->setData("application/x-myformat-doc-uid", uid);
	clipboard->setMimeData(mimeData);
}

