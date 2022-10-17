#pragma once

#include <QString>
#include <QColor>
#include <QWebElement>

class HtmlTable
{
public:
	HtmlTable();
	HtmlTable(QWebElement &table);
	~HtmlTable(void);

	int GetColCount();
	int GetRowCount();
	int GetCellCountInRow(QWebElement &tr);

	void SetDimensions(int rows, int cols);

    QString MakeHtml(int rowsCount, int colsCount);
    QString MakeHtml(const QString &text);
	QString MakeHtmlCells(int cellsCount);
	QString MakeHtmlRow(int colsCount);
    QString MakeHtmlRow(const QString &text);

	void InsertRowAbove(QWebElement &tr);
	void InsertRowBelow(QWebElement &tr);

	void InsertColLeft(QWebElement &td);
	void InsertColRight(QWebElement &td);

	void DeleteRow(QWebElement &tr);
	void DeleteColumn(QWebElement &td);

	bool NormalizeRow(QWebElement &tr);
	
protected:
	void SetTrColCount(QWebElement &tr, int colsCount);
	int  GetColIndex(QWebElement &td);
	QWebElement GetColByIndex(QWebElement &tr, int ti);
private:
	QWebElement m_table;
};
