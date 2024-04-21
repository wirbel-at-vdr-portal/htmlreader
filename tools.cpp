#include <htmlreader.h>

#include <sstream> // std::stringstream
#include <fstream> // std::ifstream


namespace HTML {

std::stringstream ReadFileToStream(std::string aFileName) {
  std::stringstream ss;
  std::ifstream is(aFileName.c_str());
  if (is) {
     ss << is.rdbuf();
     is.close();
     }
  return ss; 
}


std::string UpperCase(std::string s) {
  std::string r(s);
  std::transform(r.begin(), r.end(), r.begin(), ::tolower);
  return r;
}


Document ParseFile(std::string FileName) {
  std::stringstream ss = ReadFileToStream(FileName);
  HTML::parser HtmlParser;
  auto doc = HtmlParser.parse(ss.str());
  return doc;  
}


Node GetBody(Document doc) {
  auto tags = doc->get_elements_by_tag_name("BODY");
  if (tags.size() != 1) {
     //std::cerr << "Error: could not find html body: size == " << tags.size() << std::endl;
     return nullptr;
     }
  return tags[0];
}


Node GetNode(Node Parent, std::string Name) {
  Name = UpperCase(Name);
  for(auto node:Parent->child_nodes()) {
     if (node->name() == Name) {
        return node;
        }
     }
  return nullptr;
}


Node GetNode(Node Parent, std::string Name, std::string AttrName, std::string AttrValue) {
  Name = UpperCase(Name);
  AttrName = UpperCase(AttrName);
  for(auto node:Parent->child_nodes()) {
     if (node->name() != Name) {
        continue;
        }
     for(auto it = node->attributes_begin(); it != node->attributes_end(); it++) {
        auto attr = *it;
        if (attr->name() == AttrName and attr->value() == AttrValue) {
           return node;
           }
        }
     }
  return nullptr;  
}


std::vector<Node> GetNodeList(Node Parent, std::string Name) {
  std::vector<Node> result;
  Name = UpperCase(Name);
  for(auto node:Parent->child_nodes()) {
     if (node->name() != Name) {
        continue;
        }
     result.push_back(node);
     }
  return result;
}


std::vector<Node> GetNodeList(Node Parent, std::string Name, std::string AttrName, std::string AttrValue) {
  std::vector<Node> result;
  Name = UpperCase(Name);
  AttrName = UpperCase(AttrName);
  for(auto node:Parent->child_nodes()) {
     if (node->name() != Name) {
        continue;
        }
     for(auto it = node->attributes_begin(); it != node->attributes_end(); it++) {
        auto attr = *it;
        if (attr->name() == AttrName and attr->value() == AttrValue) {
           result.push_back(node);
           break;
           }
        }
     }
  return result;
}



} // namespace HTML
