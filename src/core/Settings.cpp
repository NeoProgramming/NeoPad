#include "Settings.h"
#include <QSettings>

static const char INI_FILE[] = "neopad.ini";

Settings    INI;

// Qt does not QVariant.getValue() method
template<typename T>
void getValue(const QVariant &v, T &x)
{
	x = v.value<T>();
}

void Settings::setDir(const QString &dir)
{
	m_dir = dir;
	QChar ch = m_dir[m_dir.count() - 1];
	if (ch != '\\' && ch != '/')
		m_dir += '/';
}

void Settings::loadSettings()
{
	QSettings settings(m_dir + INI_FILE, QSettings::IniFormat);
	
#define X(type, var, def)	getValue(settings.value(#var, def), var);
	SETTINGS_LIST
#undef X

	if (settings.allKeys().size() != 6)
		saveSettings();
}

void Settings::saveSettings()
{
	QSettings settings(m_dir + INI_FILE, QSettings::IniFormat);

#define X(type, var, def)	settings.setValue(#var, var);
	SETTINGS_LIST
#undef X
}

