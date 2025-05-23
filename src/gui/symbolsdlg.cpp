#include "symbolsdlg.h"
#include <QTextCodec>
#include <QMessageBox>
#include <QFontDatabase>
#include <QTimer>
#include <QClipboard>
#include <QMenu>

#include "../core/Solution.h"
#include "../core/ini.h"
#include "../service/tools.h"

extern QTextCodec *codecUtf8;

Q_DECLARE_METATYPE(QString *)

SymbolsDlg::SymbolsDlg(QWidget *parent)
	: QDialog(parent)
{
	ui.setupUi(this);

    ui.tableSymbols->setFocusPolicy(Qt::NoFocus);
    ui.tableSymbols->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::MinimumExpanding);
    ui.tableSymbols->verticalHeader()->setVisible(false);
    ui.tableSymbols->horizontalHeader()->setVisible(false);
    ui.tableSymbols->verticalHeader()->setMinimumSectionSize(5);
    ui.tableSymbols->horizontalHeader()->setMinimumSectionSize(5);
    ui.tableSymbols->verticalHeader()->setDefaultSectionSize(30);
    ui.tableSymbols->horizontalHeader()->setDefaultSectionSize(30);

    QTreeWidgetItem* headerItem = ui.treeGroups->headerItem();
    headerItem->setText(0, "Groups");

    m_font = ui.tableSymbols->font();

    QFontDatabase fdb;
    QStringList fonts = fdb.families();
    for(auto &fn : fonts) {
        ui.comboFonts->addItem(fn);
    }
    ui.comboFonts->setCurrentText(m_font.family());

	ui.tableSymbols->setContextMenuPolicy(Qt::CustomContextMenu);

	QAction* addAction = new QAction("Add to QuickPanel", this);
	connect(addAction, &QAction::triggered, this, &SymbolsDlg::onItemAddToQuick);
	m_contextMenu = new QMenu(this);
	m_contextMenu->addAction(addAction);
	//m_contextMenu->addAction("Delete");

    connect(ui.pushOk, &QPushButton::clicked, this, &SymbolsDlg::onOk);
    connect(ui.pushCancel, &QPushButton::clicked, this, &QDialog::reject);
    connect(ui.treeGroups, &QTreeWidget::itemClicked, this, &SymbolsDlg::onSelectGroup);
    connect(ui.tableSymbols, &QTableWidget::itemClicked, this, &SymbolsDlg::onClickSymbol);
	connect(ui.tableSymbols, &QTableWidget::itemDoubleClicked, this, &SymbolsDlg::onDoubleClickSymbol);
	connect(ui.tableSymbols, &QTableWidget::customContextMenuRequested, this, &SymbolsDlg::onRightClickSymbol);
	
    connect(ui.splitter, &QSplitter::splitterMoved, this, &SymbolsDlg::onSplitterMoved);
    connect(ui.comboFonts, &QComboBox::currentTextChanged, this, &SymbolsDlg::onFontChanged);
	connect(ui.pushSearch, &QPushButton::clicked, this, &SymbolsDlg::onSearch);
	connect(ui.pushCopy, &QPushButton::clicked, this, &SymbolsDlg::onCopy);
	connect(ui.pushAdd, &QPushButton::clicked, this, &SymbolsDlg::onAdd);
}

SymbolsDlg::~SymbolsDlg()
{

}

void SymbolsDlg::LoadLevel(QTreeWidgetItem *node, Unicode::Group *group)
{
    node->setText(0, QString::fromStdString( group->name ));
	node->setData(0, Qt::UserRole, QVariant::fromValue(group));
    for(auto &ch : group->children )
    {
        QTreeWidgetItem *item = new QTreeWidgetItem(node);
        LoadLevel(item, &ch);
    }
}

int SymbolsDlg::DoModal()
{
    ui.treeGroups->clear();
    QTreeWidgetItem *root1 = new QTreeWidgetItem();
    ui.treeGroups->addTopLevelItem(root1);
    LoadLevel(root1, &theUnicode.Recent);

    QTreeWidgetItem *root2 = new QTreeWidgetItem();
    ui.treeGroups->addTopLevelItem(root2);
    LoadLevel(root2, &theUnicode.Faves);

    QTreeWidgetItem *root3 = new QTreeWidgetItem();
    ui.treeGroups->addTopLevelItem(root3);
    LoadLevel(root3, &theUnicode.All);

	QTreeWidgetItem *root4 = new QTreeWidgetItem();
	ui.treeGroups->addTopLevelItem(root4);
	LoadLevel(root4, &theUnicode.Quick);
	
	root1->setSelected(true);
	QTimer::singleShot(0, this, SLOT(onPostInit()));

	return this->exec();
}

void SymbolsDlg::onPostInit()
{	
	LoadGroup(&theUnicode.Recent, ui.tableSymbols, m_font);
}


void SymbolsDlg::Done(QTableWidgetItem *item)
{
    if(item) {
		unsigned int c = item->data(Qt::UserRole).value<unsigned int>();
        if(ui.checkCode->checkState() == Qt::Checked) {
            m_Symbol.sprintf("&#%d;", c);
        }
        else {
            m_Symbol = item->text();
        }
		// add to recent
		theUnicode.Recent.AddRecent(c);
        accept();
    }
}

void SymbolsDlg::onOk()
{
    QTableWidgetItem *item = ui.tableSymbols->currentItem();
    Done(item);
}

void SymbolsDlg::onSelectGroup(QTreeWidgetItem *item)
{
	if (item) {
		Unicode::Group* gr = item->data(0, Qt::UserRole).value<Unicode::Group*>();
		if (gr) {
			LoadGroup(gr, ui.tableSymbols, m_font);
		}
	}
}

void SymbolsDlg::LoadGroup(Unicode::Group* gr, QTableWidget *tableSymbols, QFont &font)
{
	int row = 0;
	int col = 0;
	int count = gr->GetCount();
    int rows = (count - 1) / Q_Columns + 1;
	
	tableSymbols->clear();
	tableSymbols->setRowCount(rows);
    tableSymbols->setColumnCount(Q_Columns);
		
	for (auto i = gr->begin(), e = gr->end(); i != e; ++i) {
        if (col >= Q_Columns) {
			row++;
			col = 0;
		}
		unsigned int c = *i;
        QString s = QString::fromUcs4(&c, 1);

		QTableWidgetItem* item = new QTableWidgetItem(s);
        item->setFlags(item->flags() & ~Qt::ItemFlag::ItemIsEditable);
        item->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
        item->setToolTip(theUnicode.GetName(c));
        item->setData(Qt::UserRole, QVariant::fromValue(c));
		tableSymbols->setItem(row, col, item);
		col++;
	}
    //Make empty entries uneditable
    for(int i = col; i < Q_Columns; i++) {
       QTableWidgetItem* item = new QTableWidgetItem("");
       item->setFlags(item->flags() & ~Qt::ItemFlag::ItemIsEditable);
       tableSymbols->setItem(row, i, item);
    }
    ResizeTable(tableSymbols, font);
}

void SymbolsDlg::onClickSymbol(QTableWidgetItem *item)
{
	unsigned int c = item->data(Qt::UserRole).value<unsigned int>();
	m_Symbol += item->text();
	ui.lineSymbols->setText(m_Symbol);
}

void SymbolsDlg::onDoubleClickSymbol(QTableWidgetItem *item)
{
    Done(item);
}

void SymbolsDlg::resizeEvent(QResizeEvent* event)
{
    QDialog::resizeEvent(event);
    ResizeTable(ui.tableSymbols, m_font);
}

void SymbolsDlg::onSplitterMoved(int pos, int index)
{
    ResizeTable(ui.tableSymbols, m_font);
}

void SymbolsDlg::ResizeTable(QTableWidget *tableSymbols, QFont &font)
{
    QSize s = tableSymbols->size();
    int cw = (s.width() - 20) / Q_Columns;
    if(cw < 2)
        return;

    QHeaderView *h = tableSymbols->horizontalHeader();
    for(int i=0; i<Q_Columns; i++)
        h->resizeSection(i, cw);

    h = tableSymbols->verticalHeader();
    int nc = tableSymbols->rowCount();
    for(int i=0; i<nc; i++)
        h->resizeSection(i, cw);

    font.setPointSize(cw/2);
    tableSymbols->setFont(font);
}

void SymbolsDlg::onFontChanged(const QString &text)
{
    m_font = QFont(text);
    ResizeTable(ui.tableSymbols, m_font);
}

void SymbolsDlg::onSearch()
{
	QString text = ui.lineSearch->text();
	Unicode::Group gr;
	theUnicode.FindByName(text, gr);
	if (!gr.ranges.empty()) {
		LoadGroup(&gr, ui.tableSymbols, m_font);
	}
}

void SymbolsDlg::onCopy()
{
	QClipboard *clipboard = QApplication::clipboard();
	clipboard->setText(m_Symbol);
}

void SymbolsDlg::onAdd()
{
	QString s = ui.lineSymbols->text();
	for (QChar c : s) {
		// add to quick panel
		theUnicode.Quick.AddQuick(c.unicode());
	}	
}

void SymbolsDlg::onRightClickSymbol(const QPoint & pos)
{
	QModelIndex index = ui.tableSymbols->indexAt(pos);
	if (!index.isValid()) return;
	m_contextMenu->exec(ui.tableSymbols->mapToGlobal(pos));
}

void SymbolsDlg::onItemAddToQuick()
{
	QTableWidgetItem* currentItem = ui.tableSymbols->currentItem();
	if (currentItem != nullptr) {
		unsigned int c = currentItem->data(Qt::UserRole).value<unsigned int>();
		// add to quick panel
		theUnicode.Quick.AddQuick(c);
	}
}

