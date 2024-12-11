/*******************************************************************************
 * htmlreader, a small C++ wrapper to libxml2 html.
 * See the README file for copyright information and how to reach the author.
 ******************************************************************************/
#include "htmlreader.h"
#include <libxml/tree.h>
#include <repfunc.h> // ReplaceAll(), Trim()


Html::Attr::Attr(void) {
}

Html::Attr::Attr(void* p) {
  auto a = (xmlAttr*) p;
  auto v = xmlNodeListGetString(0, a->children, 1);

  name = (const char*) a->name;

  if (v and *v) {
     std::string s = (const char*) v;
     ReplaceAll(s, "\n", " ");
     while(s.find("  ") != std::string::npos)
        ReplaceAll(s, "  ", " ");
     value = Trim(s);
     }
  xmlFree(v);
}

std::string Html::Attr::Name(void) {
  return name;
}

std::string Html::Attr::Value(void) {
  return value;
}
