#ifndef SLNTREEWIDGET_H
#define SLNTREEWIDGET_H

#include <QTreeWidget>
#include <QDropEvent>

// for drag & drop
class SlnTreeWidget : public QTreeWidget
{
	Q_OBJECT

public:
	SlnTreeWidget(QWidget *parent = 0);
	~SlnTreeWidget();
	void dropEvent ( QDropEvent * event );
	void dragEnterEvent ( QDragEnterEvent * event );
	void dragMoveEvent(QDragMoveEvent *e);
signals:
	int dropping(QTreeWidgetItem *drag, QTreeWidgetItem *drop, int m);
protected:
	QTreeWidgetItem* m_itemDrag;
};

#endif // SLNTREEWIDGET_H
