#ifndef TOPICCHOOSER_H
#define TOPICCHOOSER_H

#include "ui_topicchooser.h"

#include <QDialog>
#include <QStringList>
#include "../core/Solution.h"

class TopicChooser : public QDialog
{
    Q_OBJECT
public:
    TopicChooser(QWidget *parent, const QString& title);
	bool DoModal(bool checks=false);
	DocItem* m_posSelected;
private slots:
	void onOk();
	void onItemChanged(QTreeWidgetItem * item, int column);
private:
	void LoadLevel(QTreeWidgetItem *treeNode, DocItem* posNode);
	void CheckSubItems(QTreeWidgetItem *node, bool check);
    Ui::TopicChooser ui;
	bool m_checks;
};

#endif // TOPICCHOOSER_H
