#ifndef MARKDOWNPARSER_H
#define MARKDOWNPARSER_H

#include <QString>
#include <QVector>
#include <QStack>
#include <QStringList>

class MarkdownParser
{
public:
    MarkdownParser();

    QString parse(const QString &markdown);

private:
    enum class State {
        Normal,         // Обычный текст
        CodeBlock,      // Внутри блока кода ```
        List,           // Внутри списка
        Table,          // Внутри таблицы
        Blockquote,     // Внутри цитаты
        Paragraph       // Внутри параграфа
    };

    struct ListContext {
        int indent;
        QString type; // "ul" или "ol"
    };

    struct TableContext {
        QList<QStringList> rows;
        bool hasAlignRow;
    };

    // Методы обработки
    void processNormalLine(const QString &line, bool isLastLine);
    void processCodeBlockLine(const QString &line);
    void processListLine(const QString &line, int spaces, const QString &trimmed);
    void processTableLine(const QString &line);
    void processBlockquoteLine(const QString &line);
    void processParagraphLine(const QString &line);

    // Вспомогательные методы
    bool isListLine(const QString &trimmed, int &spaces, QString &itemType, QString &itemContent);
    bool isTableLine(const QString &trimmed);
    bool isAlignmentRow(const QStringList &row);
    QStringList parseTableRowCells(const QString &row);
    QString convertTableToHtml();
    void closeCurrentParagraph();
    void closeCurrentList();
    void closeCurrentBlockquote();
    void closeCurrentTable();

    // Состояние парсера
    State m_state;

    // Контексты для разных состояний
    QStack<ListContext> m_listStack;
    TableContext m_tableContext;
    QStringList m_paragraphLines;
    QStringList m_blockquoteLines;

    // Для блоков кода
    QStringList m_codeBuffer;

    // Результат
    QString m_result;

    // Для отслеживания пустых строк
    int m_consecutiveEmptyLines;
};

#endif // MARKDOWNPARSER_H
