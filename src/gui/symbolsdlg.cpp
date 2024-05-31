#include "symbolsdlg.h"
#include <QTextCodec>
#include <QMessageBox>
#include <QFontDatabase>

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

    connect(ui.pushOk, &QPushButton::clicked, this, &SymbolsDlg::onOk);
    connect(ui.pushCancel, &QPushButton::clicked, this, &QDialog::reject);
    connect(ui.treeGroups, &QTreeWidget::itemClicked, this, &SymbolsDlg::onSelectGroup);
    connect(ui.tableSymbols, &QTableWidget::itemDoubleClicked, this, &SymbolsDlg::onDoubleClickSymbol);
    connect(ui.splitter, &QSplitter::splitterMoved, this, &SymbolsDlg::onSplitterMoved);
    connect(ui.comboFonts, &QComboBox::currentTextChanged, this, &SymbolsDlg::onFontChanged);
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
    //ui.splitter

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

	return this->exec();
}

void SymbolsDlg::Done(QTableWidgetItem *item)
{
    if(item) {
        if(ui.checkCode->checkState() == Qt::Checked) {
            unsigned int c = item->data(Qt::UserRole).value<unsigned int>();
            m_Symbol.sprintf("&#%d;", c);
        }
        else {
            m_Symbol = item->text();
        }

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
			LoadGroup(gr);
		}
	}
}

void SymbolsDlg::LoadGroup(Unicode::Group* gr)
{
	int row = 0;
	int col = 0;
	int count = gr->GetCount();
    int rows = (count - 1) / Q_Columns + 1;
	
	ui.tableSymbols->clear();
	ui.tableSymbols->setRowCount(rows);
    ui.tableSymbols->setColumnCount(Q_Columns);
		
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
		ui.tableSymbols->setItem(row, col, item);
		col++;
	}
    //Make empty entries uneditable
    for(int i = col; i < Q_Columns; i++) {
       QTableWidgetItem* item = new QTableWidgetItem("");
       item->setFlags(item->flags() & ~Qt::ItemFlag::ItemIsEditable);
       ui.tableSymbols->setItem(row, i, item);
    }
    ResizeTable();
}

void SymbolsDlg::onDoubleClickSymbol(QTableWidgetItem *item)
{
    Done(item);
}

void SymbolsDlg::resizeEvent(QResizeEvent* event)
{
    QDialog::resizeEvent(event);
    ResizeTable();
}

void SymbolsDlg::onSplitterMoved(int pos, int index)
{
    ResizeTable();
}

void SymbolsDlg::ResizeTable()
{
    QSize s = ui.tableSymbols->size();
    int cw = (s.width() - 20) / Q_Columns;
    if(cw < 2)
        return;

    QHeaderView *h = ui.tableSymbols->horizontalHeader();
    for(int i=0; i<Q_Columns; i++)
        h->resizeSection(i, cw);

    h = ui.tableSymbols->verticalHeader();
    int nc = ui.tableSymbols->rowCount();
    for(int i=0; i<nc; i++)
        h->resizeSection(i, cw);

    m_font.setPointSize(cw/2);
    ui.tableSymbols->setFont(m_font);
}

void SymbolsDlg::onFontChanged(const QString &text)
{
    m_font = QFont(text);
    ResizeTable();
}
