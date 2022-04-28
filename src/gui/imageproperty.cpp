#include <QFileDialog>

#include "../core/ini.h"
#include "imageproperty.h"
#include "../service/tools.h"


ImageProperties::ImageProperties(QWidget *parent)
     : QDialog(parent)
{	
	ui.setupUi(this);
	connect(ui.pushOpen,	&QPushButton::clicked,	 this, &ImageProperties::onOverview);
	connect(ui.pushCreate,  &QPushButton::clicked,   this, &ImageProperties::onCreateImage);
	connect(ui.pushEdit,    &QPushButton::clicked,   this, &ImageProperties::onEditImage);
	connect(ui.checkWidth,	&QCheckBox::clicked,	 this, &ImageProperties::onCheckWidth);
	connect(ui.checkHeight, &QCheckBox::clicked,	 this, &ImageProperties::onCheckHeight);
	connect(ui.spinWidth,   qOverload<int>(&QSpinBox::valueChanged), this, &ImageProperties::onSetWidth);
	connect(ui.spinHeight,  qOverload<int>(&QSpinBox::valueChanged), this, &ImageProperties::onSetHeight);
	connect(ui.lineFPath, &QLineEdit::textChanged, this, &ImageProperties::onPathChanged);
}

void ImageProperties::onPathChanged(const QString &path)
{
	bool p = path.contains('/');
	ui.comboAction->setEnabled(p);
	if (!p)
		ui.comboAction->setCurrentIndex(0);
}

int  ImageProperties::DoModal()
{
	ui.checkWidth->setChecked(m_width != 0);
	ui.spinWidth->setValue( abs(m_width) );
	ui.comboWidthUnit->setCurrentIndex(m_width<0 ? 1 : 0);

	ui.checkHeight->setChecked(m_height != 0);
	ui.spinHeight->setValue( abs(m_height) );
	ui.comboHeightUnit->setCurrentIndex(m_height<0 ? 1 : 0);

	ui.lineFPath->setText(m_fpath);
	
	return this->exec();
}

void ImageProperties::accept()
{	
	m_action = ui.comboAction->currentIndex();
	m_fpath = ui.lineFPath->text();

	if (!ui.checkWidth->isChecked())
		m_width = 0;
	else if (ui.comboWidthUnit->currentIndex())
		m_width = -ui.spinWidth->value();
	else
		m_width = ui.spinWidth->value();

	if (!ui.checkHeight->isChecked())
		m_height = 0;
	else if (ui.comboHeightUnit->currentIndex())
		m_height = -ui.spinHeight->value();
	else
		m_height = ui.spinHeight->value();
		
	QDialog::accept();
}

void ImageProperties::onOverview()
{
	m_fpath = QFileDialog::getOpenFileName(this, tr("Load Image..."),
							  INI::LastImageDir.c_str(),
							  tr("Images (*.png *.jpg *.gif);;All Files (*)"));
	if(!m_fpath.isEmpty())
	{
		ui.lineFPath->setText(m_fpath);
        INI::LastImageDir = U8(QDir(m_fpath).path());
	}
}

void ImageProperties::onCheckWidth(bool state)
{
	m_width = !state;

	ui.spinHeight->setEnabled(state);
	ui.spinWidth->setEnabled(state);
	ui.comboWidthUnit->setEnabled(state);
	ui.comboHeightUnit->setEnabled(state);
}

void ImageProperties::onCheckHeight(bool state)
{
	m_height = !state;

	ui.spinHeight->setEnabled(state);
	ui.spinWidth->setEnabled(state);
	ui.comboWidthUnit->setEnabled(state);
	ui.comboHeightUnit->setEnabled(state);
}

void ImageProperties::onSetHeight(int y)
{
	ui.spinHeight->setValue(y);
}

void ImageProperties::onSetWidth(int x)
{
	ui.spinWidth->setValue(x);
}

void ImageProperties::onCreateImage()
{
	// open image editor and put image path to path line
	// create empty image file and open this file
	m_fpath = GenerateUniqueFPath(m_adir, "image", "png");
	QImage image(100, 100, QImage::Format_ARGB32);
	image.fill(QColor(255, 255, 255));
	image.save(m_fpath, "PNG");
	ui.lineFPath->setText(m_fpath);
	ui.comboAction->setCurrentIndex(0);
}

void ImageProperties::onEditImage()
{
	m_fpath = ui.lineFPath->text();
	if (QFileInfo(m_fpath).isAbsolute())
		OpenInExternalApplication(this, U16(INI::ImgEditPath), m_fpath);
	else
		OpenInExternalApplication(this, U16(INI::ImgEditPath), m_adir + "/" + m_fpath);
}
