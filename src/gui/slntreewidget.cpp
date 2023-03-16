#include "slntreewidget.h"
#include <QtDebug>
#include <QTextCodec>

extern QTextCodec *codecUtf8;

SlnTreeWidget::SlnTreeWidget(QWidget *parent)
	: QTreeWidget(parent)
{
	m_itemDrag = 0;
	setAutoExpandDelay(-1);
}

SlnTreeWidget::~SlnTreeWidget()
{

}

void SlnTreeWidget::dragEnterEvent ( QDragEnterEvent * e )
{
	setAutoExpandDelay(-1);
	if(!m_itemDrag)
		m_itemDrag = currentItem();// itemAt(e->pos());
	
    qDebug() << "DragEnter: " << m_itemDrag->text(0);
	QTreeWidget::dragEnterEvent(e);
}

void SlnTreeWidget::dragMoveEvent(QDragMoveEvent *e)
{
	e->acceptProposedAction();
	QTreeView::dragMoveEvent(e);
}

void SlnTreeWidget::dropEvent ( QDropEvent * e )
{
	// drag action
	QTreeWidgetItem* target = itemAt(e->pos());
	int m = e->keyboardModifiers(); 
	int r = emit dropping(m_itemDrag, target, m);
		
	m_itemDrag = 0;
	if(!r)
		e->setDropAction(Qt::IgnoreAction);
	else
		QTreeWidget::dropEvent(e);
}



