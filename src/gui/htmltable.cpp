#include "htmltable.h"
#include "../service/tools.h"

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
	return GetCellCountInRow(tr);
}

int HtmlTable::GetCellCountInRow(QWebElement &tr)
{
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
	
	QWebElement tr = m_table.findFirst("tr");
	int rowsCount = 0;
	while (!tr.isNull()) {
		rowsCount++;
		tr = tr.nextSibling();
	}
	return rowsCount;
}

QString HtmlTable::MakeHtmlRow(const QString &text)
{
    // convert tabbed text to html row
    QString html = "<tr>";
    int start = 0;
    int end = 0;
    while(start < text.length()) {
        html += "<td>";
        end = text.indexOf('\t', start);
        if(end<0)
            end = text.length();
        html += text.mid(start, end-start);
        start = end+1;
        html += "</td>";
    }
    html += "</tr>";
    return html;
}

QString HtmlTable::MakeHtmlCells(int cellsCount)
{
	QString html;
	for (int c = 0; c < cellsCount; c++)
		html += "<td>&nbsp;</td>";
	return html;
}

QString HtmlTable::MakeHtmlRow(int colsCount)
{
	QString html = "<tr>";
	html += MakeHtmlCells(colsCount);
	html += "</tr>";
	return html;
}

QString HtmlTable::MakeHtml(const QString &text)
{
    QString html = "<table>";
    int start = 0;
    int end = 0;
    while(start < text.length()) {
        end = text.indexOf(QRegExp("[\r\n]"), start);
        if(end<0)
            end = text.length();
        html += MakeHtmlRow(text.mid(start, end-start));
        start = end+1;
        if(start < text.length() && text[start+1]=='\n')
            start++;
    }
    html += "</table>";
    return html;
}

QString HtmlTable::MakeHtml(int rowsCount, int colsCount)
{
    QString html = "<table>";
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

int HtmlTable::GetRowIndex(QWebElement &tdi)
{
	int n = 0;
	QWebElement tri = tdi.parent();
	if (tri.isNull())
		return -1;
	QWebElement tr = m_table.findFirst("tr");
	while (!tr.isNull()) {
		if (tr == tri)
			return n;
		n += 1;
		tr = tr.nextSibling();
	}
	return -1;
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

QWebElement HtmlTable::GetCellByIndex(QWebElement &tr, int ti)
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
        QWebElement td = GetCellByIndex(tr, colIndex);
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

void HtmlTable::SetFocus(QWebElement &td)
{
    if(!td.isNull()) {
       td.setFocus();
       const char *script =
               "var el = this; "
               "var range = document.createRange(); "
               "var sel = window.getSelection(); "
               "range.setStart(el, 0); "
               "range.collapse(true); "
               "sel.removeAllRanges(); "
               "sel.addRange(range); ";
        td.evaluateJavaScript(script);
    }
}

void HtmlTable::MoveRow(QWebElement &tr, bool below)
{
    if(tr.isNull())
        return;
    QWebElement tr2 = below ? tr.nextSibling() : tr.previousSibling();
    if(tr2.isNull())
        return;
    if(tr2.tagName() != "TR")
        return;
    tr.takeFromDocument();
    if(below)
        tr2.appendOutside(tr);
    else
        tr2.prependOutside(tr);

    QWebElement td = tr.findFirst("TD");
    SetFocus(td);
}

void HtmlTable::MoveColumn(QWebElement &td, bool right)
{
    if(td.isNull())
        return;

    QWebElement tri = td.parent();
    if(tri.tagName() != "TR")
        return;

    int colIndex = GetColIndex(td);
    if (colIndex < 0)
        return;
    QWebElement tr = m_table.findFirst("tr");
    while (!tr.isNull()) {
        QWebElement td = GetCellByIndex(tr, colIndex);
        if (!td.isNull()) {
            // move cell in row

            QWebElement td2 = right ? td.nextSibling() : td.previousSibling();
            if(td2.isNull())
                return;
            if(td2.tagName() != "TD")
                return;

            td.takeFromDocument();

            if(right)
                td2.appendOutside(td);
            else
                td2.prependOutside(td);
        }
        tr = tr.nextSibling();
    }

    QWebElement tdf = //tri.findFirst("TD");
            GetCellByIndex(tri, right ? colIndex+1 : colIndex-1);

    SetFocus(tdf);
}

bool HtmlTable::NormalizeRow(QWebElement &tr)
{
	if (tr.isNull())
		return false;
	int colsCount = GetColCount();
	int cellCount = GetCellCountInRow(tr);
	if (cellCount < colsCount) {
		QString cells = MakeHtmlCells(colsCount - cellCount);
		tr.appendInside(cells);
		return true;
	}
	return false;
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
        QWebElement td = GetCellByIndex(tr, colIndex);
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
        QWebElement td = GetCellByIndex(tr, colIndex);
		if (!td.isNull())
			td.appendOutside("<td>&nbsp;</td>");
		tr = tr.nextSibling();
	}
}

void HtmlTable::Parse(const QString &text, QList<QStringList> &data)
{
	int start = 0;
	int end = 0;
	while (start < text.length()) {
		end = text.indexOf(QRegExp("[\r\n]"), start);
		if (end < 0)
			end = text.length();
		QStringList sl;
		ParseRow(text.mid(start, end - start), sl);
		data.push_back(sl);		
		start = end + 1;
		if (start < text.length() && text[start + 1] == '\n')
			start++;
	}
}

void HtmlTable::ParseRow(const QString &text, QStringList &row)
{
	int start = 0;
	int end = 0;
	while (start < text.length()) {
		end = text.indexOf('\t', start);
		if (end < 0)
			end = text.length();
		row.push_back(text.mid(start, end - start));
		start = end + 1;
	}
}

int HtmlTable::Normalize(QList<QStringList> &data)
{
	int cols = 0;
	for (auto &row : data) {
		cols = std::max(cols, row.count());
	}
	for (auto &row : data) {
		while (row.count() < cols)
			row.push_back("");
	}
	return cols;
}

bool HtmlTable::RemoveEmptyRows()
{
	QWebElement tr = FirstTR();
	bool removed = false;
	while (!tr.isNull()) {
		QWebElement trn = tr.nextSibling();
		// scan row
		QWebElement td = tr.firstChild();
		bool filled = false;
		while (!td.isNull()) {
			if (!IsBlank(td.toPlainText())) {
				filled = true;
				break;
			}
			td = td.nextSibling();
		}
		if (!filled) {
			tr.removeFromDocument();
			removed = true;
		}
		tr = trn;
	}
	return removed;
}

void HtmlTable::InsertData(const QString &text, QWebElement &td)
{
	QList<QStringList> data;
	Parse(text, data);
	int cols = Normalize(data);
	int rows = data.count();

	// row & col of td
	int col = GetColIndex(td);
	int row = GetRowIndex(td);
	
	cols = std::max(col + cols, GetColCount());
	rows = std::max(row + rows, GetRowCount());
	SetDimensions(rows, cols);
	
	// insert
	QWebElement tr = td.parent();
	for (auto &r : data) {
        QWebElement td = GetCellByIndex(tr, col);
		for (auto &c : r) {
			td.setPlainText(c);
			td = td.nextSibling();
		}
		tr = tr.nextSibling();
	}
}

QWebElement HtmlTable::FirstTR()
{
	QWebElement tr = m_table.firstChild();
	if (tr.tagName() == "TBODY")
		tr = tr.firstChild();
	return tr;
}

QWebElement HtmlTable::LastTR()
{
	QWebElement tr = m_table.lastChild();
	if (tr.tagName() == "TBODY")
		tr = tr.lastChild();
	return tr;
}

void HtmlTable::AppendData(const QString &text)
{
	QWebElement tr = LastTR();
	QWebElement td = tr.firstChild();
	while (!td.isNull() && !IsBlank(td.toPlainText()))
		td = td.nextSibling();
	if(!td.isNull()) {
		td.setPlainText(text);
	}
	else {
		InsertRowBelow(tr);
		tr = LastTR();
		td = tr.firstChild();
		td.setPlainText(text);
	}
}

void HtmlTable::Expand(QWebElement &td)
{
	// iteration from end to 'td'
	QWebElement tr = LastTR();
	QWebElement tdi = tr.lastChild();
	while (tdi != td) {
		// prev item
		QWebElement tdp = tdi.previousSibling();
		if (tdp.isNull()) {
			tr = tr.previousSibling();
			tdp = tr.lastChild();
		}
		// move right
		tdi.setInnerXml(tdp.toInnerXml());
		// prev item
		tdi = tdp;
	}
	td.setPlainText("");
}

void HtmlTable::Collapse(QWebElement &td)
{
	// iteration from end to 'td'
	QWebElement tr = td.parent();
	while (!td.isNull()) {
		// next
		QWebElement tdn = td.nextSibling();
		if (tdn.isNull()) {
			tr = tr.nextSibling();
			tdn = tr.firstChild();
		}
		// move left
		td.setInnerXml(tdn.toInnerXml());
		// next
		td = tdn;
	}
}

void HtmlTable::ClearColumn(QWebElement &td)
{
	// insert
	QWebElement tri = FirstTR();
	int col = GetColIndex(td);
	while(!tri.isNull()) {
        QWebElement tdi = GetCellByIndex(tri, col);
		tdi.setPlainText("");
		tri = tri.nextSibling();
	}
}

void HtmlTable::ClearRow(QWebElement &td)
{
	QWebElement tr = td.parent();
	QWebElement tdi = tr.firstChild();
	while(!tdi.isNull()) {
		tdi.setPlainText("");
		tdi = tdi.nextSibling();
	}
}

void HtmlTable::Sort(QWebElement &tds, bool desc)
{
    if (tds.isNull())
        return;
    int colIndex = GetColIndex(tds);
    if (colIndex < 0)
        return;
    // 1. Сначала собираем все строки в массив (без извлечения)
    QVector<QWebElement> rows;
    QWebElement tr = m_table.findFirst("tr");
    while (!tr.isNull()) {
        rows.append(tr);
        tr = tr.nextSibling();
    }

    // 2. Теперь извлекаем ВСЕ строки из документа
    for (int i = 0; i < rows.size(); i++) {
        rows[i].takeFromDocument();
    }

    // 3. Сортируем извлеченные строки
    for (int i = 0; i < rows.size() - 1; i++) {
        for (int j = i+1; j < rows.size(); j++) {
            QWebElement td1 = GetCellByIndex(rows[i], colIndex);
            QWebElement td2 = GetCellByIndex(rows[j], colIndex);

            QString text1 = td1.toPlainText();
            QString text2 = td2.toPlainText();

            bool needSwap = (text1 > text2) ^ desc;

            if (needSwap) {
                auto a = rows[i];
                rows[i] =rows[j];
                rows[j] = a;
            }
        }
    }

    // 4. Вставляем отсортированные строки обратно
    QWebElement tbody = m_table.findFirst("tbody");
    QWebElement container = tbody.isNull() ? m_table : tbody;

    for (int i = 0; i < rows.size(); i++) {
        container.appendInside(rows[i]);
    }
}
