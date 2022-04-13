#include "htmltable.h"

HtmlTable::HtmlTable()
{
	m_table = QWebElement();
}

HtmlTable::HtmlTable(QWebElement &table)
{
	m_table = table;
}

HtmlTable::~HtmlTable(void)
{
}

int HtmlTable::GetColCount()
{
	if (m_table.isNull())
		return -1;
	QWebElement tr = m_table.findFirst("tr");
	if(tr.isNull())
		return -1;
	QWebElement td = tr.findFirst("td");
	int colsCount = 0;
	while (!td.isNull()) {
		colsCount += td.attribute("colspan", "1").toInt();
		td = td.nextSibling();
	}
	return colsCount;
}

int HtmlTable::GetRowCount()
{
	if (m_table.isNull())
		return -1;
	int rowsCount = 0;
	QWebElement tr = m_table.findFirst("tr");
	while (!tr.isNull()) {
		rowsCount++;
		tr = tr.nextSibling();
	}
	return rowsCount;
}

QString HtmlTable::MakeHtmlRow(int colsCount)
{
	QString html = "<tr>";
	for (int c = 0; c < colsCount; c++)
		html += "<td>&nbsp;</td>";
	html += "</tr>";
	return html;
}

QString HtmlTable::MakeHtml(int rowsCount, int colsCount)
{
	QString html;
	html.sprintf("<table>");
	for (int r=0; r<rowsCount; r++)
		html += MakeHtmlRow(colsCount);
	html += "</table>";
	return html;
}

void HtmlTable::SetTrColCount(QWebElement &tr, int colsCount)
{
	// set number of columns for <tr>
	int col = 0;
	QWebElement td = tr.findFirst("td");
	while(!td.isNull() && col < colsCount) {
		col++;
		td = td.nextSibling();
	}
	if(td.isNull()) {
		// have reached the end of the real row - insert additional columns
		while(col < colsCount) {
			tr.appendInside("<td>&nbsp;</td>");
			col++;
		}
	}
	else {
		// have reached the specified number of columns - remove unnecessary ones
		while( !td.isNull() ) {
			QWebElement tdnext = td.nextSibling();
			td.removeFromDocument();
			td = tdnext;
		}
	}
}

void HtmlTable::SetDimensions(int rowsCount, int colsCount)
{
	// adjust the number of rows and columns of the table
	// number of lines first
	QWebElement tr = m_table.findFirst("tr");
	
	// set number of columns for each line
	int row = 0;
	while(!tr.isNull() && row < rowsCount) {
		SetTrColCount(tr, colsCount);
		row++;
		tr = tr.nextSibling();
	}

	// reached the end of the real table - insert additional rows
	if(tr.isNull()) {
		QWebElement tbody = m_table.findFirst("tbody");
		if(tbody.isNull())
			tbody = m_table;
		while(row < rowsCount) {
			tbody.appendInside(MakeHtmlRow(colsCount));
			row++;
		}
	}
	// have reached the specified number of lines - remove the extra
	else  {
		while( !tr.isNull() ) {
			QWebElement trnext = tr.nextSibling();
			tr.removeFromDocument();
			tr = trnext;
		}
	}
}

int HtmlTable::GetColIndex(QWebElement &tdi)
{
	int n = 0;
	if(tdi.isNull())
		return -1;
	QWebElement tr = tdi.parent();
	if(tr.isNull())
		return -1;
	QWebElement td = tr.findFirst("td");
	while (!td.isNull()) {
		if (td == tdi)
			return n;
		n += td.attribute("colspan", "1").toInt();
		td = td.nextSibling();
	}
	return -1;
}

QWebElement HtmlTable::GetColByIndex(QWebElement &tr, int ti)
{
	QWebElement td = tr.findFirst("td");
	int i = 0;
	while (!td.isNull()) {
		if(i == ti)
			return td;
		td = td.nextSibling();
		i++;
	}
	return QWebElement();
}

void HtmlTable::DeleteColumn(QWebElement &tdi)
{
	if (tdi.isNull())
		return;
	int colIndex = GetColIndex(tdi);
	if (colIndex < 0)
		return;
	QWebElement tr = m_table.findFirst("tr");
	while (!tr.isNull()) {
		QWebElement td = GetColByIndex(tr, colIndex);
		if(!td.isNull())
			td.removeFromDocument();
		tr = tr.nextSibling();
	}
}

void HtmlTable::DeleteRow(QWebElement &tr)
{
	if(!tr.isNull())
		tr.removeFromDocument();
}

void HtmlTable::InsertRowAbove(QWebElement &tr)
{
	if(tr.isNull())
		return;
	int colsCount = GetColCount();
	tr.prependOutside(MakeHtmlRow(colsCount));
}

void HtmlTable::InsertRowBelow(QWebElement &tr)
{
	if (tr.isNull())
		return;
	int colsCount = GetColCount();
	tr.appendOutside(MakeHtmlRow(colsCount));
}

void HtmlTable::InsertColLeft(QWebElement &tdi)
{
	if (tdi.isNull())
		return;
	int colIndex = GetColIndex(tdi);
	if(colIndex<0)
		return;
	QWebElement tr = m_table.findFirst("tr");
	while (!tr.isNull()) {
		QWebElement td = GetColByIndex(tr, colIndex);
		if(!td.isNull())
			td.prependOutside("<td>&nbsp;</td>");
		tr = tr.nextSibling();
	}
}

void HtmlTable::InsertColRight(QWebElement &tdi)
{
	if (tdi.isNull())
		return;
	int colIndex = GetColIndex(tdi);
	if (colIndex < 0)
		return;
	QWebElement tr = m_table.findFirst("tr");
	while (!tr.isNull()) {
		QWebElement td = GetColByIndex(tr, colIndex);
		if (!td.isNull())
			td.appendOutside("<td>&nbsp;</td>");
		tr = tr.nextSibling();
	}
}

