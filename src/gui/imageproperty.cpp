#include <QFileDialog>
#include <QTextCodec>

#include "../core/ini.h"
#include "imageproperty.h"
#include "../service/tools.h"

extern QTextCodec *codecUtf8;



ImageProperties::ImageProperties(QWidget *parent)
     : QDialog(parent)
{	
	ui.setupUi(this);
	connect(ui.pushOpen,		SIGNAL(clicked()),		this, SLOT(onLoadImage()));	
	connect(ui.radioOriginal,	SIGNAL(clicked(bool)),	this, SLOT(onSizeOriginal(bool)));	
	connect(ui.radioCustom,		SIGNAL(clicked(bool)),	this, SLOT(onSizeCustom(bool)));	
	connect(ui.spinWidth,		SIGNAL(valueChanged(int)), this, SLOT(onSetWidth(int)));	
	connect(ui.spinHeight,		SIGNAL(valueChanged(int)), this, SLOT(onSetHeight(int)));	
	connect(ui.comboAction,     SIGNAL(currentIndexChanged(int)), this, SLOT(onActionChanged(int)));
	connect(ui.pushCreate,		SIGNAL(clicked()),		this, SLOT(onCreateImage()));
}

int  ImageProperties::DoModal(HtmlImage *image)
{
	m_image = image;
	ui.comboAction->setCurrentIndex(m_action);
	if(image)
	{
		ui.LEFileName->setEnabled(false);
		ui.comboAction->setEnabled(false);
		ui.pushOpen->setEnabled(false);

		m_sizeOrignial = 1;
		QString unit;
		int v = GetValue(image->m_width, unit);
		if(v)
			m_sizeOrignial = 0;
		ui.spinWidth->setValue( v );
		v = (unit == "%") ? 1 : 0;
		ui.comboWidthUnit->setCurrentIndex( v );
		v = GetValue(image->m_height, unit);
		if(v)
			m_sizeOrignial = 0;
		ui.spinHeight->setValue( v );
		v = (unit == "%") ? 1 : 0;
		ui.comboHeightUnit->setCurrentIndex( v );

		ui.radioCustom->setChecked(!m_sizeOrignial);
		ui.radioOriginal->setChecked(m_sizeOrignial);
		onSizeOriginal(m_sizeOrignial);
		m_fpath = image->m_fpath;
		ui.LEFileName->setText(m_fpath);
		LoadImageInfo();
	}

	return this->exec();
}

void ImageProperties::accept()
{	
	m_fpath = ui.LEFileName->text();
	if(m_image)
	{
		if(m_sizeOrignial)
		{
			m_image->m_width = "";
			m_image->m_height = "";
		}
		else
		{
			m_image->m_width.sprintf("%d%s", ui.spinWidth->value(), ui.comboWidthUnit->currentIndex() ? "%": "");
			m_image->m_height.sprintf("%d%s", ui.spinHeight->value(), ui.comboHeightUnit->currentIndex() ? "%": "");
		}
		m_image->m_src = ui.LEFileName->text();
		m_image->m_alt = ui.LEAltText->text();
		m_image->m_title = ui.LEPopUpHint->text();
	}
	QDialog::accept();
}

void ImageProperties::onLoadImage()
{
	m_fpath = QFileDialog::getOpenFileName(this, tr("Load Image..."),
							  INI::LastImageDir.c_str(),
							  tr("Images (*.png *.jpg *.gif);;All Files (*)"));
	if(!m_fpath.isEmpty())
	{
		ui.LEFileName->setText(m_fpath);
        INI::LastImageDir = U8(QDir(m_fpath).path());
		LoadImageInfo();
	}
}

void ImageProperties::LoadImageInfo()
{
	QFileInfo fi(m_fpath);
	if (fi.exists())
	{
		img.load(m_fpath);
		ui.spinHeight->setValue( img.height() );
		ui.spinWidth->setValue( img.width() );
	}
}


void ImageProperties::onSizeOriginal(bool state)
{
	onSizeCustom(!state);
}

void ImageProperties::onActionChanged(int a)
{
	m_action = a;
}


void ImageProperties::onSizeCustom(bool state)
{
	m_sizeOrignial = !state;
	ui.checkKeepRatio->setEnabled(state);
	ui.LHeight->setEnabled(state);
	ui.LWidth->setEnabled(state);
	ui.spinHeight->setEnabled(state);
	ui.spinWidth->setEnabled(state);
	ui.comboWidthUnit->setEnabled(state);
	ui.comboHeightUnit->setEnabled(state);
}


void ImageProperties::onSetHeight(int y)
{
	ui.spinHeight->setValue(y);
	if (ui.checkKeepRatio->isChecked())
	{
		QImage tmpImage = img.scaledToHeight(y, Qt::FastTransformation);
		ui.spinWidth->setValue(tmpImage.width());
	}
}


void ImageProperties::onSetWidth(int x)
{
	ui.spinWidth->setValue(x);
	if (ui.checkKeepRatio->isChecked())
	{
		QImage tmpImage = img.scaledToWidth(x, Qt::FastTransformation);
		ui.spinHeight->setValue(tmpImage.height());
	}
}

void ImageProperties::onCreateImage()
{
	// open image editor and put image path to path line
	
}

