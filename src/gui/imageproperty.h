#pragma once

#include "ui_imageproperty.h"
#include "htmlimage.h"
#include "../core/vmbsrv.h"

// insert picture
class ImageProperties : public QDialog
{
     Q_OBJECT
public:
	ImageProperties(QWidget *parent = 0);
	int  DoModal();
public:
	QString m_adir;
	ImageAction m_action = ImageAction::Href;	// copy/move/hardlink/...
	QString m_fpath;
	QString m_ext;
	int m_width=0;	// <0: percents, >0: pixels, 0: none
	int m_height=0;
	bool m_embedded = false;
	bool m_delete = false;
private slots:
	void accept();
	void onOverview();
	void onCheckWidth(bool state);
	void onCheckHeight(bool state);
	void onActionChanged(int index);
	void onPathChanged(const QString &path);
	void onSetHeight(int y);
	void onSetWidth(int x);
	void onCreateImage();
	void onEditImage();
private:

	Ui::DialogImageProperty ui;
	QImage img;
};

