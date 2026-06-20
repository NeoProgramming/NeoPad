#pragma once

#include <QString>
#include <QStringList>
#include <QVector>
#include <QPair>
#include <QChar>

struct SearchMatch {
    int startPos;           // the position of the beginning of the match in the text
    int endPos;             // match end position
    int word1Pos;           // first word position
    int word2Pos;           // second word position
    QString surroundingText; // text around for context
};

struct SearchConfig {
    bool caseSensitive = false;
    int radius = 10;         // maximum distance between words
    bool wholeWords = false; // only whole words (not implemented yet)
};


class SearchEngine
{
public:

    SearchEngine(const SearchConfig& config = SearchConfig());

    // Search for two words in text
	QVector<SearchMatch> findWordPairs(const QString& text,
		const QString& word1,
		const QString& word2) const;

	// new:
	// Возвращает список найденных групп, каждая группа - пара (начальная_позиция, конечная_позиция)
	QVector<QPair<int, int>> findWordGroups(const QString& text, 
		const QStringList& words,
		int R) const;
	// Перегруженная версия для случая M < N (ищем группы, где есть хотя бы M слов из N)
	QVector<QPair<int, int>> findWordGroups(const QString& text,
		const QStringList& words,
		int M,  // минимальное количество слов, которые должны присутствовать
		int R) const;

    // A utility for getting context around a match
	static QString extractContext(const QString& text, int pos, int contextSize = 50);

private:
    SearchConfig m_config;

    // Search all positions of a word in the text
	QVector<int> findAllPositions(const QString& text, const QString& word) const;

    // String normalization (convert to lowercase if necessary)
	QString normalize(const QString& str) const;
	// Проверка, является ли символ разделителем (пробел, пунктуация и т.д.)
	bool isWordDelimiter(QChar ch) const;
private:
	// Структура для события (позиция слова и его ID)
	struct Event {
		int position;
		int wordId;

		Event(int pos = 0, int id = 0) : position(pos), wordId(id) {}

		// Для сортировки по позиции
		bool operator<(const Event& other) const {
			return position < other.position;
		}
	};
};

