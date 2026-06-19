#include "FileExtractor.h"

#include <QFile>
#include <QTextCodec>
#include <QDebug>
#include <QRegularExpression>

QString FileExtractor::loadFile(const QString& filePath, ExtractMode mode)
{
	QFile file(filePath);
	if (!file.open(QIODevice::ReadOnly)) {
		qWarning() << "Cannot open file:" << filePath;
		return QString();
	}

	QByteArray rawData = file.readAll();
	file.close();

    // Determine the file type by extension
	if (filePath.endsWith(".mht", Qt::CaseInsensitive) ||
		filePath.endsWith(".mhtml", Qt::CaseInsensitive)) {
		return handleMhtml(rawData, mode);
	}
	else if (filePath.endsWith(".html", Qt::CaseInsensitive) ||
		filePath.endsWith(".htm", Qt::CaseInsensitive)) {
		return handleHtml(rawData, filePath, mode);
	}
	else {
        // Everything else (txt, no extension, etc.)
		return handlePlainText(rawData);
	}
}

QString FileExtractor::handleMhtml(const QByteArray& rawData, ExtractMode mode)
{
    // 1. Extracting the HTML portion from an MHTML container
	QByteArray htmlPart = extractHtmlFromMhtml(rawData);
	if (htmlPart.isEmpty()) {
		qWarning() << "No HTML content found in MHTML file";
		return QString();
	}

    // 2. Decoding quoted-printable yields raw bytes.
	QByteArray decodedBytes = decodeQuotedPrintableToBytes(htmlPart);

    // 3. Now we process it as HTML (determine the encoding)
	return handleHtml(decodedBytes, "dummy.html", mode);
}

// We decode quoted-printable into a QByteArray, not a QString.
QByteArray FileExtractor::decodeQuotedPrintableToBytes(const QByteArray& data)
{
	QByteArray result;
	result.reserve(data.size());

	QString text = QString::fromLatin1(data);
	QStringList lines = text.split('\n');

	for (int i = 0; i < lines.size(); ++i) {
		QString line = lines[i];

        // Remove \r if there is one
		if (line.endsWith('\r')) {
			line.chop(1);
		}

        // Checking if there is a soft hyphen at the end of a line
		bool softBreak = line.endsWith('=');

		if (softBreak) {
            // Remove = at the end and add without line breaks
			line.chop(1);
			result.append(processQuotedLine(line));
		}
		else {
            // Regular line - add with a line break
			result.append(processQuotedLine(line));
            if (i < lines.size() - 1) { // Don't add \n after the last line
				result.append('\n');
			}
		}
	}

	return result;
}

QByteArray FileExtractor::processQuotedLine(const QString& line)
{
	QByteArray result;
	result.reserve(line.length());

	for (int i = 0; i < line.length(); ++i) {
		QChar ch = line[i];

		if (ch == '=' && i + 2 < line.length()) {
            // Decode =XX
			QString hex = line.mid(i + 1, 2);
			bool ok;
			int value = hex.toInt(&ok, 16);

			if (ok) {
				result.append(static_cast<char>(value));
				i += 2;
			}
			else {
                // Invalid hex sequence - leave as is
				result.append('=');
			}
		}
		else {
            // A common symbol
            if (ch.toLatin1() != 0) { // Check if the character is in the ASCII range
				result.append(ch.toLatin1());
			}
			else {
                // Non-ASCII character (must be encoded with =XX)
                // If we got here, then there is something wrong with the encoding.
				result.append('?');
			}
		}
	}

	return result;
}

QString FileExtractor::handleHtml(const QByteArray& rawData, const QString& filePath, ExtractMode mode)
{
    // Determine the encoding and obtain Unicode text
	QString htmlText;

    // For HTML, we first try to determine the encoding using QTextCodec::codecForHtml
    // (it can search for meta charset and XML declaration)
	QTextCodec* codec = QTextCodec::codecForHtml(rawData);
	if (codec) {
		htmlText = codec->toUnicode(rawData);
	}
	else {
        // If that doesn't work, use detect library.
		AutoIt::Common::TextEncodingDetect detector;
		AutoIt::Common::TextEncodingDetect::Encoding encoding = detector.DetectEncoding(
			reinterpret_cast<const unsigned char*>(rawData.constData()),
			rawData.size()
		);
		htmlText = decodeWithEncoding(rawData, encoding);
	}

    // Depending on the mode, we return either full HTML or pure text.
	if (mode == ExtractTextOnly) {
		return extractPlainTextFromHtml(htmlText);
	}

	return htmlText;
}

QString FileExtractor::extractPlainTextFromHtml(const QString& html)
{
	QString plainText;
	plainText.reserve(html.size());

	bool inTag = false;
	bool inHead = false;
	bool inScript = false;
	bool inStyle = false;

	bool justClosedTag = false;
	bool lastWasSpace = false;

    // List of tags after which a line break should be added
	QStringList blockTags = { "p", "div", "h1", "h2", "h3", "h4", "h5", "h6",
							 "br", "hr", "table", "tr", "li", "section", "article" };
	

	for (int i = 0; i < html.size(); ++i) {
		QChar c = html[i];

		if (c == '<') {
			inTag = true;

            // We save the tag name for further processing.
			QString tagName;
			int j = i + 1;
			while (j < html.size() && html[j].isLetter()) {
				tagName.append(html[j]);
				j++;
			}
			tagName = tagName.toLower();

            // Let's check if this is the beginning of a special block
			if (tagName == "head") {
				inHead = true;
			}
			else if (tagName == "script") {
				inScript = true;
			}
			else if (tagName == "style") {
				inStyle = true;
			}

            // For closing tags
			if (i + 1 < html.size() && html[i + 1] == '/') {
				j = i + 2;
				tagName.clear();
				while (j < html.size() && html[j].isLetter()) {
					tagName.append(html[j]);
					j++;
				}
				tagName = tagName.toLower();

				if (tagName == "head") {
					inHead = false;
				}
				else if (tagName == "script") {
					inScript = false;
				}
				else if (tagName == "style") {
					inStyle = false;
				}
			}

			continue;
		}

		if (c == '>') {
			inTag = false;
			justClosedTag = true;
			continue;
		}

		if (inTag || inHead || inScript || inStyle) {
			continue;
		}

        // HTML entities handling (as in previous version)
		if (c == '&') {
			int semicolonPos = html.indexOf(';', i);
			if (semicolonPos > i) {
				QString entity = html.mid(i + 1, semicolonPos - i - 1);
				if (entity == "nbsp") {
					plainText.append(' ');
					lastWasSpace = true;
					justClosedTag = false;
                } // ... other entity
				i = semicolonPos;
				continue;
			}
		}

        // Inserting spaces between words
		if (justClosedTag) {
			if (!c.isSpace() && !c.isPunct() && !plainText.isEmpty() && !lastWasSpace) {
				plainText.append(' ');
				lastWasSpace = true;
			}
			justClosedTag = false;
		}

        // Handling spaces
		if (c.isSpace()) {
			if (!lastWasSpace && !plainText.isEmpty()) {
				plainText.append(' ');
				lastWasSpace = true;
			}
			while (i + 1 < html.size() && html[i + 1].isSpace()) {
				i++;
			}
			continue;
		}

		plainText.append(c);
		lastWasSpace = false;
	}

    // A simple heuristic: look for periods, colons, and increase the distance
	plainText.replace(". ", ".\n");
	plainText.replace("! ", "!\n");
	plainText.replace("? ", "?\n");
	plainText.replace(": ", ":\n");

    // We clean up unnecessary hyphens
	plainText.replace(QRegularExpression("\n{3,}"), "\n\n");

	return plainText.trimmed();
}

QString FileExtractor::handlePlainText(const QByteArray& rawData)
{
	AutoIt::Common::TextEncodingDetect detector;
	AutoIt::Common::TextEncodingDetect::Encoding encoding = detector.DetectEncoding(
		reinterpret_cast<const unsigned char*>(rawData.constData()),
		rawData.size()
	);

	QString text = decodeWithEncoding(rawData, encoding);
    // Normalize line endings to a uniform format (\n)
	text.replace("\r\n", "\n"); // Windows -> Unix
	text.replace("\r", "\n");   // Mac Classic -> Unix

	return text;
}

QString FileExtractor::decodeWithEncoding(const QByteArray& data, AutoIt::Common::TextEncodingDetect::Encoding encoding)
{
	switch (encoding) {
	case AutoIt::Common::TextEncodingDetect::ASCII:
	case AutoIt::Common::TextEncodingDetect::ANSI:
        // For ANSI/ASCII we use the local system encoding
        // In Windows, this is usually Windows-1251 or other
		return QString::fromLocal8Bit(data);

	case AutoIt::Common::TextEncodingDetect::UTF8_BOM:
	case AutoIt::Common::TextEncodingDetect::UTF8_NOBOM:
		return QString::fromUtf8(data);

	case AutoIt::Common::TextEncodingDetect::UTF16_LE_BOM:
	case AutoIt::Common::TextEncodingDetect::UTF16_LE_NOBOM:
		return QString::fromUtf16(reinterpret_cast<const char16_t*>(data.constData()),
			data.size() / 2);

	case AutoIt::Common::TextEncodingDetect::UTF16_BE_BOM:
	case AutoIt::Common::TextEncodingDetect::UTF16_BE_NOBOM: {
        // For UTF-16 BE needs to be converted to LE or use QTextCodec
		QTextCodec* codec = QTextCodec::codecForName("UTF-16BE");
		if (codec) {
			return codec->toUnicode(data);
		}
        // Fallback: Manual conversion
		QByteArray leData = data;
		char* rawData = leData.data();
		for (int i = 0; i < leData.size(); i += 2) {
			if (i + 1 < leData.size()) {
                // Swapping bytes
				char tmp = rawData[i];
				rawData[i] = rawData[i + 1];
				rawData[i + 1] = tmp;
			}
		}
		return QString::fromUtf16(reinterpret_cast<const char16_t*>(leData.constData()),
			leData.size() / 2);
	}

	case AutoIt::Common::TextEncodingDetect::None:
	default:
		qWarning() << "Unknown encoding, trying local 8-bit";
		return QString::fromLocal8Bit(data);
	}
}


// old method for backward compatibility
QString FileExtractor::decodeQuotedPrintable(const QByteArray& data)
{
	return QString::fromLatin1(decodeQuotedPrintableToBytes(data));
}


QByteArray FileExtractor::extractHtmlFromMhtml(const QByteArray& mhtmlData)
{
    // A simple MHTML parser
    // We are looking for the boundaries of the parts (boundary) and Content-Type: text/html

    QString data = QString::fromLatin1(mhtmlData); // MHTML headings are always in ASCII

    // We look for the boundary from the Content-Type header
	QRegularExpression boundaryRegex("boundary=\"?([^\";\\s]+)\"?");
	QRegularExpressionMatch boundaryMatch = boundaryRegex.match(data);

	if (!boundaryMatch.hasMatch()) {
		return QByteArray();
	}

	QString boundary = "--" + boundaryMatch.captured(1);
	QStringList parts = data.split(boundary);

	for (const QString& part : parts) {
		if (part.contains("Content-Type: text/html", Qt::CaseInsensitive)) {
            // Found the HTML part
			QStringList lines = part.split('\n');
			bool contentStarted = false;
			QByteArray html;

			for (const QString& line : lines) {
				if (!contentStarted) {
                    // Skip headers to the first empty line
					if (line.trimmed().isEmpty()) {
						contentStarted = true;
					}
				}
				else {
                    // We save it as Latin1 because quoted-printable works on bytes.
					html.append(line.toLatin1());
					html.append('\n');
				}
			}

			return html;
		}
	}

	return QByteArray();
}
