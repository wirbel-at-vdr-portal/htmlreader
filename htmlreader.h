/*******************************************************************************
 * htmlreader, a small C++ wrapper to libxml2 html.
 * See the README file for copyright information and how to reach the author.
 ******************************************************************************/
#pragma once
#include <string>
#include <vector>
#include <sstream>
#include <cstdint> // uint32_t

namespace Html {

/*******************************************************************************
 * forward decls
 ******************************************************************************/
class Attr;
class Node;
class Document;


/*******************************************************************************
 * HTML::Attr
 * The Attr class represents an attribute of an HTML tree node.
 ******************************************************************************/
class Attr {
friend class Node;
private:
  std::string name;
  std::string value;
  Attr(void* p);
public:
  Attr(void);
  std::string Name(void);
  std::string Value(void);
};


/*******************************************************************************
 * HTML::Node
 * The Node class represents an HTML tree node.
 ******************************************************************************/
enum NodeType {
   ntNull,        // Empty node
   ntDocument,    // A document tree's absolute root
   ntElement,     // Element tag, i.e. '<node/>'
   ntAttribute,   // A tree node's attribute, i.e. href="foo.bar"
   ntPcdata,      // Plain character data, i.e. 'text'
   ntCdata,       // Character data, i.e. '<![CDATA[text]]>'
   ntComment,     // Comment tag, i.e. '<!-- text -->'
   ntPi,          // Processing instruction, i.e. '<?name?>'
   ntDeclaration, // Document declaration, i.e. '<?html version="1.0"?>'
   ntDoctype      // Document type declaration, i.e. '<!DOCTYPE doc>'
};

class Node {
friend class Document;
private:
  std::string name;
  std::string rawcontent;
  int nodetype;
  int number;
  int line;
  std::vector<Node*> childs;
  std::vector<Attr*> attribs;
  Node(void* p);
public:
  Node(void);
  ~Node();
  std::string Name(void);
  std::string Content(void);
  std::string RawContent(void);
  std::vector<Node*> Children(void);
  std::vector<Attr*> Attributes(void);
  // useful tools
  Node* GetNode(std::string aName);
  Node* GetNode(std::string aName, std::string AttrName, std::string AttrValue);
  std::vector<Node*> GetNodeList(std::string aName);
  std::vector<Node*> GetNodeList(std::string aName, std::string AttrName, std::string AttrValue);
  Attr* GetAttribute(std::string aName);
  // debug functions
  int Number(void);
  int Line(void);
};


/*******************************************************************************
 * HTML::Document
 *
 * The Document class represents an HTML document and holds all resources from
 * a html page. Don't access elements or attributes after destroying it.
 ******************************************************************************/
class Document {
public:
  enum HtmlParsing {
     Relaxed               = 1,
     NoDefaultDoctype      = 4,
     SuppressErrors        = 32,
     SuppressWarnings      = 64,
     Pedantic              = 128,
     RemoveBlankNodes      = 256,
     ForbidNetworkAccess   = 2048,
     NoImpliedElements     = 8192,
     CompactSmallTextNodes = 65536,
     IgnoreEncoding        = 2097152
     };
  static const uint32_t DefaultParsing =
    Relaxed | SuppressErrors | SuppressWarnings | ForbidNetworkAccess | CompactSmallTextNodes;
private:
  void* priv;
  Node* root;
public:
  Document(std::stringstream& ss, uint32_t Options = DefaultParsing);
  ~Document();

  /* returns the root node or a nullptr.
   * DO NOT DELETE. The memory is cleaned up automatically.
   */
  Node* Root(void);
};

} /* namespace Html */
