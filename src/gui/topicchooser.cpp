#include "topicchooser.h"
#include <QTextCodec>

extern QTextCodec *codecUtf8;

TopicChooser::TopicChooser(QWidget *parent, const QString &title)
    : QDialog(parent), m_posSelected(0)
{
    ui.setupUi(this);
	connect(ui.pushOk, SIGNAL(clicked()), this, SLOT(onOk()));
	connect(ui.pushCancel, SIGNAL(clicked()), this, SLOT(reject()));
	setWindowTitle(title);
	QTreeWidgetItem* headerItem = ui.treeContents->headerItem();
	headerItem->setText(0, "Items");
}

bool TopicChooser::DoModal(bool checks)
{
	DocItem* tposRoot = theSln.GetRoot();
	if(!tposRoot) 
		return false;

	m_checks = checks;
	setCursor(Qt::WaitCursor);

	ui.treeContents->clear();
	QTreeWidgetItem *root = new QTreeWidgetItem();
	root->setText(0, tposRoot->title[0] );
	root->setData(0, Qt::UserRole, QVariant::fromValue(tposRoot));
	ui.treeContents->addTopLevelItem(root);

	LoadLevel(root, tposRoot);
	root->setExpanded(true);

	setCursor(Qt::ArrowCursor);

	if (m_checks)
		connect(ui.treeContents, SIGNAL(itemChanged(QTreeWidgetItem *, int)), this, SLOT(onItemChanged(QTreeWidgetItem *, int)));

	return exec() == QDialog::Accepted;
}

void TopicChooser::LoadLevel(QTreeWidgetItem *treeNode, DocItem* tposNode)
{
	if (m_checks)
	{
		treeNode->setFlags(treeNode->flags() | Qt::ItemIsUserCheckable);
		treeNode->setCheckState(0, Qt::Checked);
		tposNode->SetCheck(1);
	}

	for(MTPOS tpos : tposNode->children )
	{
		DocItem* docItem = tpos->This<DocItem>();
		QTreeWidgetItem *item = new QTreeWidgetItem(treeNode);
		item->setText(0, docItem->title[0]);
		item->setData(0, Qt::UserRole, QVariant::fromValue(tpos));
		LoadLevel(item, docItem);
	}
}

void TopicChooser::onOk()
{
	QTreeWidgetItem *item = ui.treeContents->currentItem();
	if(!item) return;
	m_posSelected = item->data(0, Qt::UserRole).value<DocItem*>();
	
	accept();	
}

void TopicChooser::onItemChanged(QTreeWidgetItem * item, int column)
{
	bool c = item->checkState(0);
	DocItem* pos = item->data(0, Qt::UserRole).value<DocItem*>();
	if (!pos)
		return;
	pos->SetCheck(c);
	CheckSubItems(item, c);
}

void TopicChooser::CheckSubItems(QTreeWidgetItem *node, bool check)
{
	int n = node->childCount();
	for (int i = 0; i < n; i++)
	{
		QTreeWidgetItem *item = node->child(i);
		DocItem* pos = item->data(0, Qt::UserRole).value<DocItem*>();
		pos->SetCheck(check);
		item->setCheckState(0, check ? Qt::Checked : Qt::Unchecked);
	}
}


