#include "pugitools.h"

pugi::xml_attribute set_attr(pugi::xml_node node, const char *attr_name)
{
	pugi::xml_attribute a = node.attribute(attr_name);
	if (!a)
		a = node.append_attribute(attr_name);
	return a;
}

void save_blob(const pugi::xml_document &xdoc, QByteArray &blob)
{
	struct Saver : public pugi::xml_writer {
		QByteArray &m_blob;
		Saver(QByteArray &blob) : m_blob(blob) {}
		virtual void write(const void* data, size_t size)
		{
			m_blob.append((const char*)data, size);
		}
	} saver(blob);
	xdoc.save(saver);
}


