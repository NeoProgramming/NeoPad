#include <QFileDialog>
#include "linkproperty.h"
#include "topicchooser.h"

LinkProperties::LinkProperties(QWidget *parent)
     : QDialog(parent)
{
     ui.setupUi(this);

     connect(ui.pushChooseNode, &QPushButton::clicked, this, &LinkProperties::onChooseNode);
     connect(ui.pushRemoveLink, &QPushButton::clicked, this, &LinkProperties::onRemoveLink);
}

int  LinkProperties::DoModal(bool textEditable, const QString &text, const QString &url, DocItem* item, int di)
{
	ui.lineText->setText(m_text = text);
	ui.lineUrl->setText(m_url = url);
    ui.lineText->setReadOnly(!textEditable);
	if(!textEditable)
		ui.lineText->setStyleSheet("QLineEdit {background-color: #4ce6e1;}");
	
	m_item = item;
	m_di = di;
	return this->exec();
}

void LinkProperties::onChooseNode()
{
	TopicChooser dlg(this, "Select node to link");
	if (!dlg.DoModal() || !dlg.m_posSelected)
		return;
    if (ui.lineText->text().isEmpty() && !ui.lineText->isReadOnly())
		ui.lineText->setText(dlg.m_posSelected->GetTitle(0));
	ui.lineUrl->setText(dlg.m_posSelected->GetRelUrl(m_item, m_di));
}

void LinkProperties::onRemoveLink()
{
    ui.lineUrl->setText("");
}

void LinkProperties::accept()
{
	m_text = ui.lineText->text();
	m_url = ui.lineUrl->text();
	QDialog::accept();
}
