#include "Classes.h"
#include <QTextCodec>
extern QTextCodec *codecUtf8;

QStringList* Classes::Get(const QString &name)
{
	if (!data.contains(name))
		return nullptr;
	return &data[name];
}

bool Classes::Load(pugi::xml_node txRoot)
{
	pugi::xml_node txClasses = txRoot.child("classes");
	if (!txClasses)
		return false;

	data.clear();
	pugi::xml_node txGroup = txClasses.first_child();
	while (txGroup) {
		const char *gr = txGroup.name();
		pugi::xml_node txClass = txGroup.first_child();
		QStringList cnames;
		while (txClass) {
			const char *cl = txClass.name();
			cnames.push_back(cl);
			txClass = txClass.next_sibling();
		}
		data.insert(gr, cnames);
		txGroup = txGroup.next_sibling();
	}
	return true;
}

bool Classes::Save(pugi::xml_node txRoot)
{
	pugi::xml_node xClasses = txRoot.child("classes");
	if (!xClasses)
		xClasses = txRoot.append_child("classes");
	xClasses.remove_children();
	for (auto it = data.begin(); it != data.end(); ++it) {
		QString key = it.key();
		QStringList &values = it.value();

		pugi::xml_node xGroup = xClasses.append_child(codecUtf8->fromUnicode(key).constData());
		for (const auto &n : values) {
			pugi::xml_node xClass = xGroup.append_child(codecUtf8->fromUnicode(n).constData());
		}
	}
	return true;
}

