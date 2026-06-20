#include "SearchEngine.h"
#include <QtAlgorithms>
#include <QSet>
#include <QPair>
#include <QDebug>

SearchEngine::SearchEngine(const SearchConfig& config)
	: m_config(config)
{
}

QString SearchEngine::normalize(const QString& str) const
{
	if (m_config.caseSensitive) {
		return str;
	}
	return str.toLower();
}

bool SearchEngine::isWordDelimiter(QChar ch) const
{
	// Разделители слов: пробелы, знаки препинания, символы перевода строки
	return ch.isSpace() || ch.isPunct() || ch == '\n' || ch == '\r' || ch == '\t';
}

QVector<int> SearchEngine::findAllPositions(const QString& text, const QString& word) const
{
	QVector<int> positions;

	if (word.isEmpty() || text.isEmpty()) {
		return positions;
	}

	QString searchText = normalize(text);
	QString searchWord = normalize(word);

	int pos = 0;
	while ((pos = searchText.indexOf(searchWord, pos)) != -1) {
		positions.append(pos);
        pos++; // Shift by 1 to find overlapping matches
	}

	return positions;
}

QVector<SearchMatch> SearchEngine::findWordPairs(const QString& text,
	const QString& word1,
	const QString& word2) const
{
	QVector<SearchMatch> results;

	if (word1.isEmpty() || word2.isEmpty() || text.isEmpty()) {
		return results;
	}

    // We find all positions of both words
	QVector<int> pos1 = findAllPositions(text, word1);
	QVector<int> pos2 = findAllPositions(text, word2);

	if (pos1.isEmpty() || pos2.isEmpty()) {
		return results;
	}

	int len1 = word1.length();
	int len2 = word2.length();

    // For each position of the first word, we look for a suitable second one.
	for (int p1 : pos1) {
		for (int p2 : pos2) {
            // Calculating the distance between words
			int distance = 0;
			int startPos = 0;
			int endPos = 0;

			if (p1 < p2) {
                // word 1 before word 2
				distance = p2 - (p1 + len1);
				startPos = p1;
				endPos = p2 + len2;
			}
			else {
                // word 2 before word 1
				distance = p1 - (p2 + len2);
				startPos = p2;
				endPos = p1 + len1;
			}

            // Checking the radius
			if (distance <= m_config.radius) {
				SearchMatch match;
				match.startPos = startPos;
				match.endPos = endPos;
				match.word1Pos = p1;
				match.word2Pos = p2;
				match.surroundingText = extractContext(text, startPos, 50);

				results.append(match);
			}
		}
	}

    // Remove duplicates (if both words are the same)
    // Easy sorting and uniqueness
	std::sort(results.begin(), results.end(),
		[](const SearchMatch& a, const SearchMatch& b) {
		if (a.startPos != b.startPos) return a.startPos < b.startPos;
		return a.endPos < b.endPos;
	});

	auto last = std::unique(results.begin(), results.end(),
		[](const SearchMatch& a, const SearchMatch& b) {
		return a.startPos == b.startPos && a.endPos == b.endPos;
	});
	results.erase(last, results.end());

	return results;
}

QString SearchEngine::extractContext(const QString& text, int pos, int contextSize)
{
	int start = qMax(0, pos - contextSize);
	int end = qMin(text.length(), pos + contextSize);

	QString context = text.mid(start, end - start);

    // Add ellipses if necessary.
	if (start > 0) context = "..." + context;
	if (end < text.length()) context = context + "...";

    // Replace line breaks with spaces for compactness
	context.replace('\n', ' ').replace('\r', ' ');

	return context;
}

QVector<QPair<int, int>> SearchEngine::findWordGroups(const QString& text,
	const QStringList& words,
	int R) const
{
	// Вызываем версию с M = words.size() (нужны все слова)
	return findWordGroups(text, words, words.size(), R);
}

QVector<QPair<int, int>> SearchEngine::findWordGroups(const QString& text,
	const QStringList& words,
	int M,  // минимальное количество слов, которые должны присутствовать
	int R) const
{
	QVector<QPair<int, int>> result;

	if (text.isEmpty() || words.isEmpty() || M <= 0 || R < 0) {
		return result;
	}

	// Проверяем, что M не больше количества слов
	int N = words.size();
	if (M > N) {
		qWarning() << "M cannot be greater than N";
		return result;
	}

	// 1. Собираем позиции всех слов
	QVector<QVector<int>> allPositions;
	for (const QString& word : words) {
		QVector<int> positions = findAllPositions(text, word);
		if (positions.isEmpty()) {
			// Если слово не найдено, а M == N, то группа невозможна
			if (M == N) {
				return result;
			}
			// Если M < N, просто добавляем пустой список
		}
		allPositions.append(positions);
	}

	// 2. Создаем единый массив событий
	QVector<Event> events;
	for (int id = 0; id < allPositions.size(); ++id) {
		for (int pos : allPositions[id]) {
			events.append(Event(pos, id));
		}
	}

	// 3. Сортируем события по позиции
	std::sort(events.begin(), events.end());

	// 4. Скользящее окно
	QVector<int> count(N, 0);  // счетчик вхождений каждого слова в окне
	int found = 0;              // количество уникальных слов в окне
	int left = 0;
	int right = 0;

	while (right < events.size()) {
		// Добавляем правое событие
		Event& eRight = events[right];
		if (count[eRight.wordId] == 0) {
			found++;
		}
		count[eRight.wordId]++;

		// Если нашли достаточно слов, пытаемся сузить окно
		while (found >= M && left <= right) {
			Event& eLeft = events[left];
			Event& eRightRef = events[right];

			// Проверяем условие близости
			if (eRightRef.position - eLeft.position <= R) {
				// Нашли валидную группу
				result.append(qMakePair(eLeft.position, eRightRef.position));

				// Если нужно найти все группы, а не только минимальные,
				// можно продолжить поиск. Но обычно находят минимальные окна.
				// Для поиска всех групп, раскомментируйте следующую строку:
				// break; // Останавливаем сужение для текущего right
			}

			// Удаляем левое событие
			count[eLeft.wordId]--;
			if (count[eLeft.wordId] == 0) {
				found--;
			}
			left++;
		}

		right++;
	}

	// 5. Опционально: фильтруем дубликаты (если группа найдена несколько раз)
	// Удаляем дублирующиеся группы (одинаковые пары позиций)
	QSet<QPair<int, int>> uniqueGroups;
	for (const auto& group : result) {
		uniqueGroups.insert(group);
	}
	result.clear();
	for (const auto& group : uniqueGroups) {
		result.append(group);
	}

	// Сортируем результат по начальной позиции
	std::sort(result.begin(), result.end(),
		[](const QPair<int, int>& a, const QPair<int, int>& b) {
		return a.first < b.first;
	});

	return result;
}

