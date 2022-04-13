#pragma once

#include "../3rdparty/pugixml/pugixml.hpp"
#include <QByteArray>

pugi::xml_attribute set_attr(pugi::xml_node node, const char *attr_name);
void save_blob(const pugi::xml_document &xdoc, QByteArray &blob);

