#include "savealldlg.h"
#include "webeditview.h"
#include "../core/Solution.h"
#include "mainwindow.h"

extern QTextCodec *codecUtf8;

SaveAllDlg::SaveAllDlg(QWidget *parent)
	: QDialog(parent)
{
	ui.setupUi(this);

	connect(ui.pushSaveAll,		SIGNAL(clicked()), this, SLOT(onSaveAll()));
	connect(ui.pushSaveNothing, SIGNAL(clicked()), this, SLOT(onSaveNothing()));
	connect(ui.pushOk,			SIGNAL(clicked()), this, SLOT(onSaveSel()));
	connect(ui.pushCancel,		SIGNAL(clicked()), this, SLOT(reject()));
}

SaveAllDlg::~SaveAllDlg()
{

}

bool SaveAllDlg::DoModal(MainWindow *mw)
{
	// list of unsaved vmbase
	for (DocItem* mtpos : mpl)
	{
		QListWidgetItem *item = new QListWidgetItem( mtpos->GetTitle(0));
		item->setData(Qt::UserRole, QVariant::fromValue(mtpos));
		item->setData(Qt::UserRole+1, QVariant::fromValue(0));
		item->setIcon(mw->m_iconVmb);
		
		ui.listFiles->addItem(item);
		item->setSelected(1);
	}

	// list of open documents
	QList<QMdiSubWindow *>::Iterator i = wl.begin();
	QList<QMdiSubWindow *>::Iterator e = wl.end();
	while(i != e)
	{
		QMdiSubWindow *subwnd = *i;
		WebEditView *view = qobject_cast<WebEditView *>(subwnd->widget());
		if(view->isWindowModified())
		{
			QListWidgetItem *item = new QListWidgetItem(view->windowTitle());
			item->setData(Qt::UserRole, QVariant::fromValue(view));
			item->setData(Qt::UserRole+1, QVariant::fromValue(1));
			item->setIcon(mw->m_iconHtm);
			
			ui.listFiles->addItem(item);
			item->setSelected(1);
		}
		++i;
	}
	ui.listFiles->setFocus();
	if(ui.listFiles->count() > 0 )
		return this->exec();
	return QDialog::Accepted;
}

void SaveAllDlg::DoSave(bool all, bool save)
{
	int n = ui.listFiles->count();
	for(int i=0; i<n; i++)
	{
		QListWidgetItem *item = ui.listFiles->item(i);
		if(all || item->isSelected())
		{
			// see what kind of element it is; 
			unsigned int t = item->data(Qt::UserRole+1).value<unsigned int>();
			if(t == 0)	// this is a vmbase node
			{
				if (save) {
					DocItem* tpos = item->data(Qt::UserRole).value<DocItem*>();
					theSln.SaveSubBase(tpos, false);
				}
			}
			else if(t == 1) // this is html document
			{
				WebEditView *view = item->data(Qt::UserRole).value<WebEditView*>();
				if (save)
					view->onFileSave();
				else
					view->m_DontSaveBeforeClosing = true;
			}
		}
	}
}

void SaveAllDlg::onSaveNothing()
{
	DoSave(true, false);
	accept();
}

void SaveAllDlg::onSaveAll()
{
	DoSave(true, true);
	accept();
}

void SaveAllDlg::onSaveSel()
{
	DoSave(false, true);
	accept();
}


