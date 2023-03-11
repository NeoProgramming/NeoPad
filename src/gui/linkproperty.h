#ifndef __LINKPROPERTY_H__
#define __LINKPROPERTY_H__

#include "ui_linkproperty.h"
#include "../core/DocItem.h"

class LinkProperties : public QDialog
{
     Q_OBJECT
public:
	QString m_text, m_url;

public:
	LinkProperties(QWidget *parent = 0);
    int DoModal(bool textEditable, const QString &text, const QString &url, DocItem* item, int di);

public slots:
    void onChooseNode();
    void onRemoveLink();
	void accept();
	
private:
	Ui::LinkProperty ui;
	DocItem* m_item;
	int m_di;
};

#endif // __LINKPROPERTY_H__
