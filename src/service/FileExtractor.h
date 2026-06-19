#pragma once

#include <QString>
#include <QByteArray>
#include "../3rdparty/ted/text_encoding_detect.h"

class FileExtractor
{
public:
	enum ExtractMode {
        ExtractFull,      // Full HTML (for display)
        ExtractTextOnly   // Text only (for searching)
	};

    // Main method: Loads a file and returns Unicode text
	static QString loadFile(const QString& filePath, ExtractMode mode = ExtractFull);
private:
    // Auxiliary methods
	static QString decodeQuotedPrintable(const QByteArray& data);
	static QString decodeWithEncoding(const QByteArray& data, AutoIt::Common::TextEncodingDetect::Encoding encoding);
	static QByteArray decodeQuotedPrintableToBytes(const QByteArray& data);
private:
    // Handlers for different formats
	static QString handleMhtml(const QByteArray& rawData, ExtractMode mode);
	static QString handleHtml(const QByteArray& rawData, const QString& filePath, ExtractMode mode);
	static QString handlePlainText(const QByteArray& rawData);
    static QByteArray processQuotedLine(const QString& line);

    // Searching for HTML content in MHTML
	static QByteArray extractHtmlFromMhtml(const QByteArray& mhtmlData);

	static QString extractPlainTextFromHtml(const QString& html);
};

