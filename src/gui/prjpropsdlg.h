#ifndef PRJPROPSDLG_H
#define PRJPROPSDLG_H

#include <QDialog>
#include "ui_prjpropsdlg.h"
#include "../core/Solution.h"

class PrjPropsDlg : public QDialog
{
	Q_OBJECT

public:
	PrjPropsDlg(QWidget *parent = 0);
	~PrjPropsDlg();
	int DoModal();

public slots:
	void onOk();
	void onOverviewSnippets();
	void onOverviewImages();

	void onOverviewBPath1();
	void onOverviewBPath2();
	void onOverviewBCSS1();
	void onOverviewBCSS2();
public:

	QString m_images;
	QString m_snippets;
	NeopadBook m_bases[BCNT];
private:
	Ui::PrjPropsDlg ui;
};

#endif // PRJPROPSDLG_H
