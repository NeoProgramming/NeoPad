#pragma once

#include "ui_imageproperty.h"
#include "htmlimage.h"

// insert picture
class ImageProperties : public QDialog
{
     Q_OBJECT
	HtmlImage *m_image;
public:
	ImageProperties(QWidget *parent = 0);
	int  DoModal(HtmlImage *table);

	int  m_action;
	QString m_fpath;
	bool m_sizeOrignial;
public slots:
	void onLoadImage();

private slots:
	void accept();
	void onSizeOriginal(bool state);
	void onSizeCustom(bool state);

	void onSetHeight(int y);
	void onSetWidth(int x);
	void onActionChanged(int a);
	void onCreateImage();
private:
	void LoadImageInfo();

	Ui::DialogImageProperty ui;
	QImage img;
	QString previewImageFN, projectImageFN, strHtml;
};

