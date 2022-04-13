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

QTreeWidgetItem *SlnTreeWidget::GetPrevSibling(QTreeWidgetItem *item)
{
	QTreeWidgetItem *parent = item->parent();
	QTreeWidgetItem *sibling;
	if(parent)
		sibling = parent->child(parent->indexOfChild(item)-1);
	else 
		sibling = topLevelItem(indexOfTopLevelItem(item)-1);
	return sibling;
}

QTreeWidgetItem *SlnTreeWidget::GetNextSibling(QTreeWidgetItem *item)
{
	QTreeWidgetItem *parent = item->parent();
	QTreeWidgetItem *sibling;
	if(parent)
		sibling = parent->child(parent->indexOfChild(item)+1);
	else 
		sibling = topLevelItem(indexOfTopLevelItem(item)+1);
	return sibling;
}

void SlnTreeWidget::MoveItem(QTreeWidgetItem *item, QTreeWidgetItem *insparent, QTreeWidgetItem *insafter)
{
	// move item element - Child of insparent, after insafter (or to the end if null)
	QTreeWidgetItem *oldparent = item->parent();
	if(oldparent)
	{
		// take
		int oldindex = oldparent->indexOfChild(item);
		QTreeWidgetItem *i = oldparent->takeChild(oldindex); // should be i == item

		// put in a new place
		if(insafter)
		{
			int insindex = insparent->indexOfChild(insafter) + 1;
			insparent->insertChild(insindex, item);
		}
		else
		{
			int insindex = insparent->childCount();
			insparent->insertChild(insindex, item);
		}
	}
}
