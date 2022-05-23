#include "htmlimage.h"
#include <QFile>
#include <QFileInfo>

HtmlImage::HtmlImage()
{
	m_image = QWebElement();
}

HtmlImage::HtmlImage(QWebElement &image)
{
	m_image = image;
}

HtmlImage::~HtmlImage(void)
{
}

QString HtmlImage::GetSrc()
{
	if (m_image.isNull())
		return QString();
	return m_image.attribute("src");
}

int HtmlImage::GetWidth()
{
	if (m_image.isNull())
		return 0;
	QString width = m_image.attribute("width");
	if (width.endsWith('%'))
		return -width.mid(0, width.length()-1).toInt();
	return width.toInt();
}

int HtmlImage::GetHeight()
{
	if (m_image.isNull())
		return 0;
	QString height = m_image.attribute("height");
	if (height.endsWith('%'))
		return -height.mid(0, height.length() - 1).toInt();
	return height.toInt();
}

bool HtmlImage::IsEmbedded()
{
	// <img src="data:
	QString src = m_image.attribute("src");
	if (src.startsWith("data:", Qt::CaseInsensitive))
		return true;
	return false;
}

void HtmlImage::SetSrc(const QString &fpath)
{
	m_image.setAttribute("src", fpath);
}

void HtmlImage::SetWidth(int w)
{
	if(w < 0)
		m_image.setAttribute("width", QString::asprintf("%d%%", -w));
	else if(w > 0)
		m_image.setAttribute("width", QString::asprintf("%d", w));
	else
		m_image.removeAttribute("width");
}

void HtmlImage::SetHeight(int h)
{
	if (h < 0)
		m_image.setAttribute("height", QString::asprintf("%d%%", -h));
	else if (h > 0)
		m_image.setAttribute("height", QString::asprintf("%d", h));
	else
		m_image.removeAttribute("height");
}

QString HtmlImage::ConvertToEmbedded(const QString &fpath)
{
	QString res = "data:image;base64,";
	QFile file(fpath);
	if (!file.open(QIODevice::ReadOnly))
		return "";
	QByteArray data = file.readAll();
	file.close();
	res += data.toBase64();
	return res;
}

QString HtmlImage::GetImageExt(const QString& code)
{
	// gif87a:  47 49 46 38 37 61
	// gif89a:  47 49 46 38 39 61
	// png:		89 50 4E 47 0D 0A 1A 0A
	// jpg:		FF D8 FF DB
	// jpg:		FF D8 FF E0 ?? ?? 4A 46 49 46 00 01
	// jpg:		FF D8 FF E1 ?? ?? 45 78 69 66 00 00
	// webp:	52 49 46 46 ?? ?? ?? ?? 57 45 42 50

	if (code.startsWith("data:image/png;base64,"))
		return "png";
	if (code.startsWith("data:image/jpg;base64,"))
		return "jpg";
	if (code.startsWith("data:image/gif;base64,"))
		return "gif";

	if (!code.startsWith("data:image;base64,"))
		return "";

	QByteArray data = QByteArray::fromBase64(code.mid(18).toLocal8Bit());
	const char *p = data.constData();
	if (p[0] == 0x47 && p[1] == 0x49 && p[2] == 0x46 && p[3] == 0x38 && p[4] == 0x37 && p[5] == 0x61)
		return "gif";
	if (p[0] == 0x47 && p[1] == 0x49 && p[2] == 0x46 && p[3] == 0x38 && p[4] == 0x39 && p[5] == 0x61)
		return "gif";
	if (p[1] == 0x50 && p[2] == 0x4E && p[3] == 0x47 && p[4] == 0x0D && p[5] == 0x0A && p[6] == 0x1A && p[7] == 0x0A)
		return "png";
	if (p[0] == 0xFF && p[1] == 0xD8 && p[2] == 0xFF && p[3] == 0xDB)
		return "jpg";
	if (p[0] == 0xFF && p[1] == 0xD8 && p[2] == 0xFF && p[3] == 0xE0 && p[6] == 0x4A && p[7] == 0x46 && p[8] == 0x49 && p[9] == 0x46 && p[10] == 0x00 && p[11] == 0x01)
		return "jpg";
	if (p[0] == 0xFF && p[1] == 0xD8 && p[2] == 0xFF && p[3] == 0xE1 && p[6] == 0x45 && p[7] == 0x78 && p[8] == 0x69 && p[9] == 0x66 && p[10] == 0x00 && p[11] == 0x00)
		return "jpg";
	if (p[0] == 0x52 && p[1] == 0x49 && p[2] == 0x46 && p[3] == 0x46 && p[8] == 0x57 && p[9] == 0x45 && p[10] == 0x42 && p[11] == 0x50)
		return "webp";
	return "";
}

bool HtmlImage::ConvertToFile(const QString& code, const QString &fpath)
{

	if (!code.startsWith("data:image;base64,"))
		return false;
	QFile file(fpath);
	if (!file.open(QIODevice::WriteOnly))
		return false;
	QByteArray data = QByteArray::fromBase64(code.mid(18).toLocal8Bit());

	file.write(data);
	file.close();
	return true;
}
