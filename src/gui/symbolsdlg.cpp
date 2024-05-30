#include "symbolsdlg.h"
#include <QTextCodec>
#include <QMessageBox>

#include "../core/Solution.h"
#include "../core/ini.h"
#include "../service/tools.h"

extern QTextCodec *codecUtf8;

Q_DECLARE_METATYPE(QString *)

SymbolsDlg::SymbolsDlg(int cols, QWidget *parent)
	: QDialog(parent)
    , m_cols(cols)
{
	ui.setupUi(this);

//    QFont table_font = Globals::fonts[0];
//    table_font.setPointSize(12);
//    ui.tableSymbols->setFont(table_font);
    ui.tableSymbols->setFocusPolicy(Qt::NoFocus);
    ui.tableSymbols->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::MinimumExpanding);
//    ui.tableSymbols->setMaximumHeight(60);
//    ui.tableSymbols->setMinimumWidth(30 * m_cols + 28);
    ui.tableSymbols->verticalHeader()->setVisible(false);
    ui.tableSymbols->horizontalHeader()->setVisible(false);
    ui.tableSymbols->verticalHeader()->setMinimumSectionSize(5);
    ui.tableSymbols->horizontalHeader()->setMinimumSectionSize(5);
    ui.tableSymbols->verticalHeader()->setDefaultSectionSize(30);
    ui.tableSymbols->horizontalHeader()->setDefaultSectionSize(30);

    QTreeWidgetItem* headerItem = ui.treeGroups->headerItem();
    headerItem->setText(0, "Groups");

	connect(ui.pushOk, SIGNAL(clicked()), this, SLOT(onOk()));
	connect(ui.pushCancel, SIGNAL(clicked()), this, SLOT(reject()));
	connect(ui.pushEdit, SIGNAL(clicked()), this, SLOT(onEdit()));
    connect(ui.treeGroups, &QTreeWidget::itemClicked, this, &SymbolsDlg::onSelect);
}

SymbolsDlg::~SymbolsDlg()
{

}

void SymbolsDlg::LoadLevel(QTreeWidgetItem *node, Unicode::Group *group)
{
    node->setText(0, QString::fromStdString( group->name ));
    for(auto ch : group->children )
    {
        QTreeWidgetItem *item = new QTreeWidgetItem(node);
        //item->setData(0, Qt::UserRole, QVariant::fromValue(tpos));
        LoadLevel(item, &ch);
    }
}

int SymbolsDlg::DoModal()
{
    ui.treeGroups->clear();
    QTreeWidgetItem *root1 = new QTreeWidgetItem();
    root1->setText(0, "Recent" );
    ui.treeGroups->addTopLevelItem(root1);
    //  LoadLevel(root1, &theUnicode.recent);

    QTreeWidgetItem *root2 = new QTreeWidgetItem();
    root2->setText(0, "Favorites" );
    ui.treeGroups->addTopLevelItem(root2);
    //  LoadLevel(root2, &theUnicode.Faves);

    QTreeWidgetItem *root3 = new QTreeWidgetItem();
    root3->setText(0, "All" );
    ui.treeGroups->addTopLevelItem(root3);
    LoadLevel(root3, &theUnicode.All);

    //	QListWidgetItem *item = new QListWidgetItem();
    //	item->setText(s);
    //	item->setData(Qt::UserRole, QVariant::fromValue(&s));
    //	ui.listSnippets->addItem(item);

	return this->exec();
}

void SymbolsDlg::onOk()
{
    QTreeWidgetItem *item = ui.treeGroups->currentItem();
	if(item)
	{
		//m_pSel = item->data(Qt::UserRole).value<QString*>();
		accept();
	}
}

void SymbolsDlg::onEdit()
{
	// open the selected snippet in an external editor
    QTreeWidgetItem *item = ui.treeGroups->currentItem();
	if(item)
	{
        QString *path = item->data(0, Qt::UserRole).value<QString *>();
		QString  cmd = codecUtf8->toUnicode(INI::HtmEditPath.c_str());
		OpenInExternalApplication(this, cmd, *path);
	}
}

void SymbolsDlg::onSelect(QTreeWidgetItem *item)
{
	onOk();
}
