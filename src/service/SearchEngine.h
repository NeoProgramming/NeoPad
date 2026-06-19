#pragma once

#include <QString>
#include <QStringList>
#include <QVector>
#include <QPair>

struct SearchMatch {
    int startPos;           // the position of the beginning of the match in the text
    int endPos;             // match end position
    int word1Pos;           // first word position
    int word2Pos;           // second word position
    QString surroundingText; // text around for context
};

struct SearchConfig {
    bool caseSensitive = false;
    int radius = 50;         // maximum distance between words
    bool wholeWords = false; // only whole words (not implemented yet)
};


class SearchEngine
{
public:

    SearchEngine(const SearchConfig& config = SearchConfig());

    // Search for two words in text
	QVector<SearchMatch> findTwoWords(const QString& text,
		const QString& word1,
		const QString& word2) const;

    // A utility for getting context around a match
	static QString extractContext(const QString& text, int pos, int contextSize = 50);

private:
    SearchConfig m_config;

    // Search all positions of a word in the text
	QVector<int> findAllPositions(const QString& text, const QString& word) const;

    // String normalization (convert to lowercase if necessary)
	QString normalize(const QString& str) const;
};

