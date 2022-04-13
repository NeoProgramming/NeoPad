#include "Snippets.h"
#include <QDir>
#include "vmbsrv.h"

void Snippets::LoadSnippets()
{
	m_SnippList.clear();
	QDir dir(m_SnippDir);
	QStringList lst = dir.entryList(QStringList("*.html"), QDir::Files);
	for (auto i : lst) {
		m_SnippList.push_back(i);
	}
}

QString Snippets::GetSnippetPath(const QString& name)
{
	QString path = m_SnippDir;
	path += "/";
	path += name;
	path += MBA::extHtml;
	return path;
}

QString Snippets::GetSnippet(const QString& name)
{
	// open the snippet, read its code
	QString s;
	QString path = GetSnippetPath(name);

	QFile file(path);
	if (file.open(QIODevice::ReadOnly))
	{
		while (!file.atEnd()) {
			QByteArray line = file.readLine();
			s += line;
			s += "\r\n";
		}
		file.close();
	}

	return s;
}

