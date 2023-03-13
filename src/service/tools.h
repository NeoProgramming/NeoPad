#pragma once
#include <string>
#include <QString>
#include <QToolBar>
#include <QTreeWidgetItem>

bool IsBlank(const QString &s);
void AddSlash(QString &path);
void NormalizeFName(QString &c);
unsigned int GenerateUniqueFNum(const QString& path, const QString& name, const QString& ext);
QString	GenerateUniqueFTitle(const QString& path, const QString& name, const QString& ext);
QString	GenerateUniqueFName(const QString& path, const QString& name, const QString& ext);
QString	GenerateUniqueFPath(const QString& path, const QString& name, const QString& ext);

QString U16(const char* s);
QString U16(const std::string &s);
std::string U8(const QString &s);
QByteArray U8a(const QString &s);

bool OpenInExternalApplication(QWidget *par, const QString &app, const QString& FileName);
int  StartExternalApplication(const QString &app, const QString &FileName, const QString& dir);
int  StartExternalApplication(const QString &app, QStringList &arguments, const QString& dir);
void SetAutoRaiseToolBar( QToolBar *toolbar, bool value );

QString GetRelPath(const QString &path, const QString &base, bool base_is_file);
QString GetRelDir(const QString &path, const QString &base, bool base_is_file);

void RemoveTreeNode(QTreeWidgetItem *item);
