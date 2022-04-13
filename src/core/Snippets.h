#pragma once
#include <QString>
#include <QStringList>

class Snippets
{
public:
	QStringList m_SnippList;// list of snippets in the current directory
	QString m_SnippDir;		// path to directory with html snippets
public:
	void	LoadSnippets();
	QString GetSnippetPath(const QString& name);
	QString GetSnippet(const QString& name);
};
