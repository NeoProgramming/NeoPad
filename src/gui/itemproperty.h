#ifndef __ITEMPROPERTY_H__
#define __ITEMPROPERTY_H__

#include "ui_itemproperty.h"
#include "../core/PrjItem.h"

//==================== class ItemProperties ====================
class ItemProperties : public QDialog
{
     Q_OBJECT

public:
    ItemProperties(QWidget *parent);
    int DoModal(MTPOS tpos);
	QString m_id, m_title0, m_title1;

signals:
	void insertContentsItem(QString title, QString fileName, QString iconFN);
	void updateContentsItem(QString title, QString fileName, QString iconFN);
     
public slots:
	void onOk();		
private:
	Ui::ItemProperty ui;
	
}; // class ItemProperties

#endif // __ITEMPROPERTY_H__
