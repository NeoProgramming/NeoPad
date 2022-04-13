#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QTextStream>
#include <QMessageBox>
#include <QDebug>
#include <QTextCodec>

#include "itemproperty.h"
#include "../core/Solution.h"

extern QTextCodec *codecUtf8;


ItemProperties::ItemProperties(QWidget *parent)
     : QDialog(parent)
{
     ui.setupUi(this);

	 connect(ui.pushOk, SIGNAL(clicked()), this, SLOT(onOk()));
	 connect(ui.pushCancel, SIGNAL(clicked()), this, SLOT(reject()));
}

int ItemProperties::DoModal(MTPOS tpos)
{
	ui.lineTitle0->setText(m_title0);
	ui.lineTitle1->setText(m_title1);

	ui.lineID->setText(tpos->GetId());
	ui.lineDoc0Path->setText(tpos->GetDocAbsPath(0));
	ui.lineDoc1Path->setText(tpos->GetDocAbsPath(1));
	ui.lineXmlPath->setText(tpos->GetVmbAbsPath());

	ui.lineInfo->setText(tpos->GetInfo());
	ui.lineSrvInfo->setText(tpos->GetInfo2());
	ui.lineTime0->setText(tpos->GetDocTimeStr(0));
	ui.lineTime1->setText(tpos->GetDocTimeStr(1));

	ui.lineParent->setText(tpos->parent ? tpos->parent->id : "");
    for(MTPOS tchild : tpos->children)
    {
		ui.listChildren->addItem(tchild->GetId());
    }

	return this->exec();
}

void ItemProperties::onOk()
{
	m_title0 = ui.lineTitle0->text();
	m_title1 = ui.lineTitle1->text();

	accept();
}

