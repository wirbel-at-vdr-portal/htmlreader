/*******************************************************************************
 * htmlreader, a small C++ wrapper to libxml2 html.
 * See the README file for copyright information and how to reach the author.
 ******************************************************************************/
#include "htmlreader.h"
#include <iostream>
#include <libxml/tree.h>
#include <repfunc.h> // ReplaceAll(), Trim()

int num=0;

Html::Node::Node(void) : nodetype(0), number(0), line(0) {
}

Html::Node::Node(void* p) {
  auto ptr = (xmlNode*) p;

  auto s = ptr->name;
  if (s and *s)
     name = (const char*) s;
  else
     name = "(null)";

  s = ptr->content;
  if (s and *s) {
     rawcontent = (const char*) s;
     }

  int types[] = {
    0,
    ntElement,   // XML_ELEMENT_NODE
    ntAttribute, // XML_ATTRIBUTE_NODE
    ntPcdata,    // XML_TEXT_NODE
    ntCdata,     // XML_CDATA_SECTION_NODE
    ntNull,      // XML_ENTITY_REF_NODE
    0,           // XML_ENTITY_NODE, unused
    ntPi,        // XML_PI_NODE
    ntComment,   // XML_COMMENT_NODE
    ntDocument,  // XML_DOCUMENT_NODE
    0,           // XML_DOCUMENT_TYPE_NODE, unused
    ntDocument,  // XML_DOCUMENT_FRAG_NODE
    0,           // XML_NOTATION_NODE, unused
    ntDocument,  // XML_HTML_DOCUMENT_NODE
    ntNull,      // XML_DTD_NODE
    ntElement,   // XML_ELEMENT_DECL
    ntAttribute, // XML_ATTRIBUTE_DECL
    ntNull,      // XML_ENTITY_DECL
    ntNull,      // XML_NAMESPACE_DECL
    ntNull,      // XML_XINCLUDE_START
    ntNull,      // XML_XINCLUDE_END
  };
  nodetype = types[ptr->type];
  number = num++;
  line = ptr->line;

  for(auto a=ptr->properties; a; a=a->next) {
     auto attr = new Attr(a);
     attribs.push_back(attr);
     }
  for(auto c=ptr->children; c; c=c->next) {
     auto child = new Node(c);
     childs.push_back(child);
     }
}

Html::Node::~Node() {
  for(auto& c:childs) {
     delete c;
     c = nullptr;
     }
  for(auto& a:attribs) {
     delete a;
     a = nullptr;
     }
  childs.clear();
  attribs.clear();
}

std::string Html::Node::Name(void) {
  return name;
}

std::string Html::Node::Content(void) {
  std::string content(rawcontent);
  ReplaceAll(content, "\n", " ");
  while(content.find("  ") != std::string::npos)
     ReplaceAll(content, "  ", " ");
  return Trim(content);
}

std::string Html::Node::RawContent(void) {
  return rawcontent;
}

std::vector<Html::Node*> Html::Node::Children(void) {
  return childs;
}

std::vector<Html::Attr*> Html::Node::Attributes(void) {
  return attribs;
}

int Html::Node::Number(void) {
  return number;
}

int Html::Node::Line(void) {
  return line;
}

Html::Node* Html::Node::GetNode(std::string aName) {
  aName = LowerCase(aName);
  for(auto node:childs) {
     if (LowerCase(node->Name()) == aName) {
        return node;
        }
     }
  return nullptr;
}

Html::Node* Html::Node::GetNode(std::string aName, std::string AttrName, std::string AttrValue) {
  aName = LowerCase(aName);
  AttrName = LowerCase(AttrName);
  AttrValue = LowerCase(AttrValue);
  for(auto node:childs) {
     if (LowerCase(node->Name()) != aName) {
        continue;
        }
     for(auto attr:node->attribs) {
        if (LowerCase(attr->Name()) == AttrName and LowerCase(attr->Value()) == AttrValue)
           return node;
        }
     }
  return nullptr;
}

std::vector<Html::Node*> Html::Node::GetNodeList(std::string aName) {
  std::vector<Html::Node*> result;
  aName = LowerCase(aName);
  for(auto node:childs) {
     if (LowerCase(node->Name()) != aName) {
        continue;
        }
     result.push_back(node);
     }
  return result;
}

std::vector<Html::Node*> Html::Node::GetNodeList(std::string aName, std::string AttrName, std::string AttrValue) {
  std::vector<Html::Node*> result;
  aName = LowerCase(aName);
  AttrName = LowerCase(AttrName);
  AttrValue = LowerCase(AttrValue);
  for(auto node:childs) {
     if (LowerCase(node->Name()) != aName) {
        continue;
        }
     for(auto attr:node->attribs) {
        if (LowerCase(attr->Name()) == AttrName and LowerCase(attr->Value()) == AttrValue) {
           result.push_back(node);
           break;
           }
        }
     }
  return result;
}

Html::Attr* Html::Node::GetAttribute(std::string aName) {
  aName = LowerCase(aName);
  for(auto attr:attribs) {
     if (LowerCase(attr->Name()) == aName)
        return attr;
     }
  return nullptr;
}
