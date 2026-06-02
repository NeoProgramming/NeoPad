#include "markdownparser.h"
#include <QRegularExpression>
#include <QTextStream>

MarkdownParser::MarkdownParser()
    : m_state(State::Normal)
    , m_consecutiveEmptyLines(0)
{
}

QString MarkdownParser::parse(const QString &markdown)
{
    // Нормализация переносов строк
    QString normalized = markdown;
    normalized.replace("\r\n", "\n");
    normalized.replace("\r", "\n");

    QTextStream stream(&normalized, QIODevice::ReadOnly);
    stream.setAutoDetectUnicode(true);

    m_result.clear();
    m_state = State::Normal;
    m_listStack.clear();
    m_paragraphLines.clear();
    m_blockquoteLines.clear();
    m_codeBuffer.clear();
    m_consecutiveEmptyLines = 0;

    QString line;
    QVector<QString> lines;

    // Сначала читаем все строки
    while (!stream.atEnd()) {
        lines.append(stream.readLine());
    }

    // Обрабатываем каждую строку
    for (int i = 0; i < lines.size(); ++i) {
        QString line = lines[i];
        bool isLastLine = (i == lines.size() - 1);

        switch (m_state) {
            case State::Normal:
                processNormalLine(line, isLastLine);
                break;
            case State::CodeBlock:
                processCodeBlockLine(line);
                break;
            case State::List:
                {
                    int spaces = 0;
                    QString trimmed = line;
                    while (spaces < trimmed.length() && trimmed[spaces] == ' ') {
                        spaces++;
                    }
                    trimmed = trimmed.trimmed();

                    QString itemType, itemContent;
                    if (isListLine(trimmed, spaces, itemType, itemContent)) {
                        processListLine(line, spaces, trimmed);
                    } else {
                        // Строка не принадлежит списку
                        closeCurrentList();
                        processNormalLine(line, isLastLine);
                    }
                }
                break;
            case State::Table:
                if (isTableLine(line.trimmed())) {
                    processTableLine(line);
                } else {
                    closeCurrentTable();
                    processNormalLine(line, isLastLine);
                }
                break;
            case State::Blockquote:
                if (line.trimmed().startsWith(">")) {
                    processBlockquoteLine(line);
                } else {
                    closeCurrentBlockquote();
                    processNormalLine(line, isLastLine);
                }
                break;
            case State::Paragraph:
                if (line.trimmed().isEmpty()) {
                    closeCurrentParagraph();
                    m_consecutiveEmptyLines++;
                    if (m_consecutiveEmptyLines == 1) {
                        m_result += "\n";
                    }
                } else {
                    m_consecutiveEmptyLines = 0;
                    processParagraphLine(line);
                }
                break;
        }
    }

    // Закрываем все незакрытые блоки
    closeCurrentParagraph();
    closeCurrentList();
    closeCurrentBlockquote();
    closeCurrentTable();

    return m_result;
}

void MarkdownParser::processNormalLine(const QString &line, bool isLastLine)
{
    QString trimmed = line.trimmed();

    // Пустая строка
    if (trimmed.isEmpty()) {
        m_consecutiveEmptyLines++;
        if (m_consecutiveEmptyLines == 1) {
            m_result += "\n";
        }
        return;
    }

    m_consecutiveEmptyLines = 0;

    // Проверка на начало блока кода
    if (trimmed.startsWith("```")) {
        m_state = State::CodeBlock;
        m_codeBuffer.clear();
        return;
    }

    // Проверка на заголовки
    QRegularExpression headerRegex("^(#{1,6})\\s+(.+)$");
    QRegularExpressionMatch match = headerRegex.match(trimmed);
    if (match.hasMatch()) {
        int level = match.captured(1).length();
        QString content = match.captured(2);
        m_result += QString("<h%1>%2</h%1>\n").arg(level).arg(content);
        return;
    }

    // Проверка на горизонтальную линию
    if (trimmed == "---" || trimmed == "***" || trimmed == "___") {
        m_result += "<hr>\n";
        return;
    }

    // Проверка на начало списка
    int spaces = 0;
    QString itemType, itemContent;
    if (isListLine(trimmed, spaces, itemType, itemContent)) {
        m_state = State::List;
        processListLine(line, spaces, trimmed);
        return;
    }

    // Проверка на начало таблицы
    if (isTableLine(trimmed)) {
        m_state = State::Table;
        m_tableContext.rows.clear();
        m_tableContext.hasAlignRow = false;
        processTableLine(line);
        return;
    }

    // Проверка на цитату
    if (trimmed.startsWith(">")) {
        m_state = State::Blockquote;
        processBlockquoteLine(line);
        return;
    }

    // Обычный параграф
    m_state = State::Paragraph;
    processParagraphLine(line);
}

void MarkdownParser::processCodeBlockLine(const QString &line)
{
    QString trimmed = line.trimmed();

    if (trimmed == "```") {
        // Конец блока кода
        if (!m_codeBuffer.isEmpty()) {
            m_result += "<pre>";
            for (int i = 0; i < m_codeBuffer.size(); ++i) {
                if (i > 0) m_result += "\n";
                m_result += m_codeBuffer[i];
            }
            m_result += "</pre>\n";
        }
        m_state = State::Normal;
    } else {
        // Содержимое блока кода
        m_codeBuffer.append(line);
    }
}

void MarkdownParser::processListLine(const QString &line, int spaces, const QString &trimmed)
{
    QString itemType, itemContent;
    int dummySpaces = spaces;
    isListLine(trimmed, dummySpaces, itemType, itemContent);

    // Обработка вложенности
    if (m_listStack.isEmpty()) {
        m_result += QString("<%1>\n").arg(itemType);
        m_listStack.push({spaces, itemType});
    } else if (spaces > m_listStack.top().indent) {
        m_result += QString("<%1>\n").arg(itemType);
        m_listStack.push({spaces, itemType});
    } else if (spaces < m_listStack.top().indent) {
        while (!m_listStack.isEmpty() && m_listStack.top().indent > spaces) {
            m_result += QString("</%1>\n").arg(m_listStack.top().type);
            m_listStack.pop();
        }

        if (!m_listStack.isEmpty() && m_listStack.top().indent == spaces &&
            m_listStack.top().type != itemType) {
            m_result += QString("</%1>\n").arg(m_listStack.top().type);
            m_listStack.pop();
            m_result += QString("<%1>\n").arg(itemType);
            m_listStack.push({spaces, itemType});
        } else if (m_listStack.isEmpty() || m_listStack.top().indent < spaces) {
            m_result += QString("<%1>\n").arg(itemType);
            m_listStack.push({spaces, itemType});
        }
    } else if (spaces == m_listStack.top().indent && m_listStack.top().type != itemType) {
        m_result += QString("</%1>\n").arg(m_listStack.top().type);
        m_listStack.pop();
        m_result += QString("<%1>\n").arg(itemType);
        m_listStack.push({spaces, itemType});
    }

    // Обработка форматирования внутри элемента списка
    QString formattedContent = itemContent;
    formattedContent = formattedContent.replace(QRegularExpression("\\*\\*(.*?)\\*\\*"), "<strong>\\1</strong>");
    formattedContent = formattedContent.replace(QRegularExpression("__(.*?)__"), "<strong>\\1</strong>");
    formattedContent = formattedContent.replace(QRegularExpression("\\*(.*?)\\*"), "<em>\\1</em>");
    formattedContent = formattedContent.replace(QRegularExpression("_(.*?)_"), "<em>\\1</em>");
    formattedContent = formattedContent.replace(QRegularExpression("`([^`]+)`"), "<code>\\1</code>");
    formattedContent = formattedContent.replace(QRegularExpression("\\[(.*?)\\]\\((.*?)\\)"), "<a href=\"$2\">$1</a>");

    m_result += QString("  <li>%1</li>\n").arg(formattedContent);
}

void MarkdownParser::processTableLine(const QString &line)
{
    QString trimmed = line.trimmed();
    QStringList cells = parseTableRowCells(trimmed);

    if (m_tableContext.rows.isEmpty()) {
        // Первая строка - заголовки
        m_tableContext.rows.append(cells);
    } else if (!m_tableContext.hasAlignRow && isAlignmentRow(cells)) {
        // Строка выравнивания
        m_tableContext.hasAlignRow = true;
    } else {
        // Строка данных
        m_tableContext.rows.append(cells);
    }
}

void MarkdownParser::processBlockquoteLine(const QString &line)
{
    QString content = line.trimmed();
    if (content.startsWith(">")) {
        content = content.mid(1).trimmed();
    }

    if (!content.isEmpty()) {
        m_blockquoteLines.append(content);
    }
}

void MarkdownParser::processParagraphLine(const QString &line)
{
    m_paragraphLines.append(line.trimmed());
}

bool MarkdownParser::isListLine(const QString &trimmed, int &spaces, QString &itemType, QString &itemContent)
{
    if (trimmed.startsWith("* ")) {
        itemType = "ul";
        itemContent = trimmed.mid(2);
        return true;
    }
    else if (trimmed.startsWith("- ")) {
        itemType = "ul";
        itemContent = trimmed.mid(2);
        return true;
    }
    else if (trimmed.contains(QRegularExpression("^\\d+\\.\\s"))) {
        itemType = "ol";
        int dotPos = trimmed.indexOf('.');
        itemContent = trimmed.mid(dotPos + 1).trimmed();
        return true;
    }

    return false;
}

bool MarkdownParser::isTableLine(const QString &trimmed)
{
    return trimmed.startsWith('|') && trimmed.endsWith('|');
}

bool MarkdownParser::isAlignmentRow(const QStringList &row)
{
    if (row.isEmpty()) return false;

    for (const QString &cell : row) {
        for (int i = 0; i < cell.size(); ++i) {
            QChar ch = cell[i];
            if (ch != ':' && ch != '-' && ch != ' ') {
                return false;
            }
        }
    }
    return true;
}

QStringList MarkdownParser::parseTableRowCells(const QString &row)
{
    QString trimmed = row.trimmed();

    // Убираем первый и последний |
    if (trimmed.startsWith('|')) {
        trimmed = trimmed.mid(1);
    }
    if (trimmed.endsWith('|')) {
        trimmed = trimmed.left(trimmed.length() - 1);
    }

    QStringList cells = trimmed.split('|');
    for (QString &cell : cells) {
        cell = cell.trimmed();
    }

    return cells;
}

QString MarkdownParser::convertTableToHtml()
{
    if (m_tableContext.rows.isEmpty()) return "";

    QStringList headers = m_tableContext.rows[0];
    int dataStartIndex = m_tableContext.hasAlignRow ? 2 : 1;

    QString html = "<table>\n  <tr>\n";
    for (const QString &header : headers) {
        html += QString("    <th>%1</th>\n").arg(header);
    }
    html += "  </tr>\n";

    for (int i = dataStartIndex; i < m_tableContext.rows.size(); ++i) {
        html += "  <tr>\n";
        for (const QString &cell : m_tableContext.rows[i]) {
            html += QString("    <td>%1</td>\n").arg(cell);
        }
        html += "  <tr>\n";
    }

    html += "</table>\n";
    return html;
}

void MarkdownParser::closeCurrentParagraph()
{
    if (!m_paragraphLines.isEmpty()) {
        QString paragraph = m_paragraphLines.join(" ");

        // Обработка форматирования внутри параграфа
        paragraph = paragraph.replace(QRegularExpression("\\*\\*(.*?)\\*\\*"), "<strong>\\1</strong>");
        paragraph = paragraph.replace(QRegularExpression("__(.*?)__"), "<strong>\\1</strong>");
        paragraph = paragraph.replace(QRegularExpression("\\*(.*?)\\*"), "<em>\\1</em>");
        paragraph = paragraph.replace(QRegularExpression("_(.*?)_"), "<em>\\1</em>");
        paragraph = paragraph.replace(QRegularExpression("`([^`]+)`"), "<code>\\1</code>");
        paragraph = paragraph.replace(QRegularExpression("\\[(.*?)\\]\\((.*?)\\)"), "<a href=\"$2\">$1</a>");

        m_result += QString("<p>%1</p>\n").arg(paragraph);
        m_paragraphLines.clear();
    }
    m_state = State::Normal;
}

void MarkdownParser::closeCurrentList()
{
    while (!m_listStack.isEmpty()) {
        m_result += QString("</%1>\n").arg(m_listStack.top().type);
        m_listStack.pop();
    }
    m_state = State::Normal;
}

void MarkdownParser::closeCurrentBlockquote()
{
    if (!m_blockquoteLines.isEmpty()) {
        m_result += "<blockquote>\n";
        for (const QString &line : m_blockquoteLines) {
            m_result += line + "\n";
        }
        m_result += "</blockquote>\n";
        m_blockquoteLines.clear();
    }
    m_state = State::Normal;
}

void MarkdownParser::closeCurrentTable()
{
    if (!m_tableContext.rows.isEmpty()) {
        m_result += convertTableToHtml();
        m_tableContext.rows.clear();
    }
    m_state = State::Normal;
}
