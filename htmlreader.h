#pragma once

#include <string>        // std::string, std::wstring
#include <vector>        // std::vector
#include <memory>        // std::shared_ptr
#include <list>          // std::list
#include <functional>    // std::function
#include <stdexcept>     // std::runtime_error
#include <unordered_map> // std::unordered_map

namespace HTML {

/*******************************************************************************
 * global config
 ******************************************************************************/
  // Uncomment this to enable wchar_t mode
  //#define HTML_WCHAR_MODE

  #ifdef HTML_WCHAR_MODE
     typedef wchar_t Char;
     typedef std::wstring String;
  #else
     typedef char Char;
     typedef std::string String;
  #endif


/*******************************************************************************
 * forward decls
 ******************************************************************************/
  class attribute;
  class document;
  class node;
  class node_walker;
  class parser;
  class parse_error;
  class token_iterator;


/*******************************************************************************
 * global type defs
 ******************************************************************************/

  /**
   * The Document definition.
   */
  typedef std::shared_ptr<document> Document;

  /**
   * The Node definition
   */
  typedef std::shared_ptr<node> Node;

  /**
   * The Attribute definition
   */
  typedef std::shared_ptr<attribute> Attribute;

/*******************************************************************************
 * HTML::node
 ******************************************************************************/

/**
 * The node class represents an HTML document tree node.
 */
class node : public std::enable_shared_from_this<node> {
public:
  /**
   * Tree node types.
   */
  enum node_type {
     node_null,        // Empty (null) node handle
     node_document,    // A document tree's absolute root
     node_element,     // Element tag, i.e. '<node/>'
     node_attribute,   // A tree node's attribute
     node_pcdata,      // Plain character data, i.e. 'text'
     node_cdata,       // Character data, i.e. '<![CDATA[text]]>'
     node_comment,     // Comment tag, i.e. '<!-- text -->'
     node_pi,          // Processing instruction, i.e. '<?name?>'
     node_declaration, // Document declaration, i.e. '<?html version="1.0"?>'
     node_doctype      // Document type declaration, i.e. '<!DOCTYPE doc>'
     };

  /**
   * Node children interator type.
   */
  typedef std::list<Node>::iterator iterator;

  /**
   * Node attribute interator type.
   */
  typedef std::list<Attribute>::iterator attribute_iterator;


public:
  /**
   * Constructs node with specified type, default is pcdata, aka text.
   */
  static Node create(node_type type = node_pcdata);

  /**
   * @return node type.
   */
  node_type type() const;

  /**
   * @return node name, e.g. HTML or BODY, etc.
   */
  String name() const;

  /**
   * Sets node tag name. Node name is optional, e.g. pcdata nodes
   * do not have a name.
   */
  void name(const String& name);

  /**
   * @return text inside node.
   */
  String value() const;

  /**
   * Change text node value.
   */
  void value(const String& value);

  /**
   * @return the textual content of the specified node, and all its
   *  descendants.
   */
  String text_content() const;


  //---------- Attribute related methods ----------

  /**
   * Returns pointer to the first attribute or nullptr, if node has
   * no attributes.
   */
  Attribute first_attribute() const;

  /**
   * Returns pointer to the first attribute or nullptr, if node has
   * no attributes.
   */
  Attribute last_attribute() const;

  /**
   * @return pointer to the attribute with the specified name or nullptr,
   * if such attribute does not exist.
   */
  Attribute get_attribute(const String& name) const;

  /**
   * Appends new attribute with the specified name to the end of attribute
   * list.
   *
   * @param name new attribute name.
   * @param value new attribute value.
   * @return pointer to newly added attribute.
   */
  Attribute append_attribute(const String& name, const String& value = "");

  /**
   * Appends new attribute to the end of attribute list.
   *
   * @return pointer to newly added attribute.
   */
  Attribute append_attribute(Attribute attr);

  /**
   * Prepends new attribute with the specified name to the beginning of
   * attribute list.
   *
   * @param name new attribute name.
   * @param value new attribute value.
   * @return pointer to newly added attribute.
   */
  Attribute prepend_attribute(const String& name, const String& value = "");

  /**
   * Prepends new attribute to the beginning of attribute list.
   *
   * @return pointer to newly added attribute.
   */
  Attribute prepend_attribute(Attribute attr);

  /**
   * Remove specified attribute if it exists.
   *
   * @return true on success, false if attribute with the specified name
   *  was not found.
   */
  bool remove_attribute(const String& name);

  /**
   * Find attribute using predicate. Returns first attribute for which
   * predicate returned true.
   */
  template <typename T> Attribute find_attribute(T pred) const;


  //---------- Child nodes related methods ----------

  /**
   * @return first child node or nullptr, if no such exist.
   */
  Node first_child() const;

  /**
   * @return first child node or nullptr, if no such exist.
   */
  Node last_child() const;

  /**
   * @return pointer to the child node with the specified name or nullptr,
   *  if such does not exist.
   */
  Node child(const String& name) const;

  /**
   * @return next node in the children list of the parent node.
   */
  Node next_sibling() const;

  /**
   * @return next sibling node with the specified name or nullptr, if
   *  such node does not exist.
   */
  Node next_sibling(const String& name) const;

  /**
   * @return previous the children list of the parent node.
   */
  Node previous_sibling() const;

  /**
   * @return previous sibling node with the specified name or nullptr, if
   *  such node does not exist.
   */
  Node previous_sibling(const String& name) const;

  /**
   * @return parent node or nullptr, if there's no parent.
   */
  Node parent() const;

  /**
   * @return root of DOM tree this node belongs to.
   */
  Node root() const;

  /**
   * Returns empty string, if no PCDATA/CDATA child nodes are found.
   *
   * @return value of the first child node of type PCDATA/CDATA.
   */
  String child_value() const;

  /**
   * @return value of the first child node with the specified name.
   */
  String child_value(const String& name) const;

  /**
   * Append new child node.
   */
  void append_child(Node aNode);

  /**
   * Prepend new child node.
   */
  void prepend_child(Node aNode);

  /**
   * Remove child node with the specified name.
   *
   * @return true on success, false if such node was not found.
   */
  bool remove_child(const String& name);

  /**
   * @return immutable list of child nodes.
   */
  const std::list<Node>& child_nodes() const;

  /**
   * Find child node using predicate. Returns first child for which
   * predicate returned true.
   */
  template <typename T> Node find_child(T pred) const;

  /**
   * Find node from subtree using predicate. Returns first node from
   * subtree (depth-first), for which predicate returned true.
   *
   * @cond INTERNAL
   */
  template <typename T> Node find_node(T pred) const {
     Node child = this->first_child();
     int traverse_depth = 1;

     while(traverse_depth > 0) {
        if (pred(child))
           return child;
        if (child->first_child()) {
           child = child->first_child();
           ++traverse_depth;
           }
        else if (child->next_sibling())
           child = child->next_sibling();
        else {
           while(!child->next_sibling() && traverse_depth > 0) {
              child = child->parent();
              --traverse_depth;
              }
           if (traverse_depth > 0)
              child = child->next_sibling();
           }
        }

     return Node(nullptr);
     }
  /* @endcond */

  /**
   * Finds all nodes satisfying the specified predicate.
   */
  std::list<Node> find_nodes(std::function<bool (Node)> predicate) const;

  /**
   * Find child node by attribute name/value. Checks only the specified
   * tag nodes.
   * Checks only child nodes. Does not traverse deeper levels.
   *
   * @param tag node tag name to check for attribute.
   * @param attr_name attribute name to check value for.
   * @param attr_value expected attribute value.
   * @return html node with the specified tag name and attribute or
   *  empty node if the specified criteria were not satisfied.
   */
  Node find_child_by_attribute(const String& tag,
                               const String& attr_name,
                               const String& attr_value) const;

  /**
   * Find child node by attribute name/value. Checks only child nodes.
   * Does not traverse deeper levels.
   *
   * @param attr_name attribute name to check value for.
   * @param attr_value expected attribute value.
   * @return html node with the specified tag name and attribute or
   *  empty node if the specified criteria were not satisfied.
   */
  Node find_child_by_attribute(const String& attr_name,
                               const String& attr_value) const;

  /**
   * Get the absolute node path from root as a text string.
   */
  String path(Char delimiter = '/') const;

  /**
   * Search for a node by path consisting of node names.
   */
  Node first_element_by_path(const String& path,
                             Char delimiter = '/') const;

  /**
   * Recursively traverse subtree with node_walker.
   *
   * @return same as walker.end() return value.
   */
  bool traverse(node_walker& walker);

  /**
   * Traverses node tree applying the specified predicate for each
   * node.
   *
   * @param predicate a C++ style predicate.
   * @param depth traversing depth.
   * @return true if whole tree was traversed, false if traversing was
   *  terminated.
   */
  bool traverse(std::function<bool (Node)> predicate,
                std::size_t depth = 0);

  /**
   * @return node children begin iterator pointing to the first child
   *  or end() iterator if this node has no children nodes.
   */
  iterator begin();

  /**
   * @return node children iterator refering to the past the last
   *  child node.
   */
  iterator end();

  /**
   * @return an iterator to the first attribute of this node.
   */
  attribute_iterator attributes_begin();

  /**
   * @return the attributes end iterator.
   */
  attribute_iterator attributes_end();

  /**
   * Converts DOM tree back to html string format starting from the
   * children of current node.
   */
  String to_string(std::size_t indentation = 0) const;

protected:
  /**
   * Creates node with the specified type.
   * Prevents from creating non-shared node.
   */
  node(node_type type = node_pcdata);

private:
  std::weak_ptr<node> parent_;
  // Iterator in parent child nodes. Used for next_sibling(), prev_sibling().
  iterator parent_it_;

  String name_;
  String value_;
  node_type type_;

  std::list<Node> children_;
  std::list<Attribute> attributes_;
};


/*******************************************************************************
 * @cond INTERNAL
 *
 * The node_walker class is an internal abstract DOM tree node
 * walker class (see node::traverse)
 ******************************************************************************/

typedef std::shared_ptr<node_walker> NodeWalker;

class node_walker {
  friend class node;

public:
  node_walker();
  virtual ~node_walker();

  /**
   * Callback that is called when traversal begins. Always returns true.
   * Override to change
   *
   * @return false if should stop iterating the tree.
   */
  virtual bool begin(Node node);

  /**
   * Callback that is called for each node traversed
   *
   * @return false if should stop iterating the tree.
   */
  virtual bool for_each(Node node) = 0;

  /**
   * Callback that is called when traversal ends.
   *
   * @return traversal state: success or failure.
   */
  virtual bool end(Node node);

protected:
  /**
   * @return current traversal depth.
   */
  int depth() const;

private:
  int depth_;
};

typedef std::function<bool (Node, std::size_t)>
  node_walker_callback;

/**
 * Makes node_walker object which implements only for_each method from the
 * specified function object.
 */
NodeWalker make_node_walker(node_walker_callback for_each);

/* @endcond */


/*******************************************************************************
 * HTML::attribute
 ******************************************************************************/

/**
 * Attribute class, a tree nodes attribute.
 */
class attribute : public node {
public:
  /**
   * Constructs attribute with default value "".
   */
  static Attribute create(const String& name, const String& value = "");

  /**
   * Compares attribute name/value pairs.
   */
  bool operator==(const attribute& attr) const;

  /**
   * Compares attribute name/value pairs.
   */
  bool operator!=(const attribute& attr) const;

  /**
   * Set attribute value.
   */
  attribute& operator=(const String& attr_val);

  /**
   * @return attribute name or "" if attribute is empty.
   */
  String name() const;

  /**
   * @return attribute value or "" if attribute is empty.
   */
  String value() const;

  /**
   * Sets attribute value.
   */
  void value(const String& attr_val);

private:
  attribute(const String& name, const String& value);
  String name_;
  String value_;
};


/*******************************************************************************
 * HTML::document
 ******************************************************************************/

/**
 * Document class (DOM tree root).
 */
class document : public node {
public:
  /**
   * Builds an empty document. It's html node with type node_document.
   */
  static Document create();

  /**
   * Returns an array of all the links in the current document.
   * The links collection counts 'a href=""' tags and 'area' tags.
   */
  std::vector<Node> links() const;

  /**
   * Traverses DOM tree and searches for html node with the specified
   * id attribute. If no tag is found, empty html node is returned.
   */
  Node get_element_by_id(const String& id) const;

  /**
   * @return a list of alls elements with the specified tag name.
   */
  std::vector<Node> get_elements_by_tag_name(const String& tag_name) const;

private:
  /**
   * Builds an empty document. It's html node with type node_document.
   */
  document();
};


/*******************************************************************************
 * HTML::parser
 ******************************************************************************/

/**
 * HTML parser.
 *
 * For testing purposes some functions are exposed publicly. Like DOCTYPE parsing.
 * DOCTYPE consists of nested sections of the following possible types:
 * 1. <!-- ... -->, <? ... ?>, "...", '...'
 * 2. <![...]]>
 * 3. <!...>
 * First group can not contain nested groups.
 * Second group can contain nested groups of the same type.
 * Third group can contain all other groups.
 *
 * NOTE:
 * Actualy HTML standard only includes the third group <!DOCTYPE ...>. So the
 * first two groups parsing might be removed in the future.
 */
class parser {
public:
  /**
   * chartype_t is shared between parser and tokenizer.
   */
  enum chartype_t {
     ct_parse_pcdata = 1,     // Parse until \0, <
     ct_parse_attr = 2,       // Parse until \0, \r, \t, \n, >, space
     ct_parse_attr_ws = 4,    // Parse until \0, &, \r, ', ", \n, tab
     ct_space = 8,            // Parse until \r, \n, space, tab
     ct_parse_cdata = 16,     // Parse until \0, ], >, \r
     ct_parse_comment = 32,   // Parse until \0, -, >, \r
     ct_symbol = 64,          // Any symbol > 127, a-z, A-Z, 0-9, _, :, -, .
     ct_start_symbol = 128    // Any symbol > 127, a-z, A-Z, _, :
     };

  /**
   * Parsing status, shared betweeen parser and parse_error.
   */
  enum parse_status {
     status_ok = 0,              // No error
     status_file_not_found,      // File was not found during load_file()
     status_io_error,            // Error reading from file/stream
     status_out_of_memory,       // Could not allocate memory
     status_internal_error,      // Internal error occurred
     status_unrecognized_tag,    // Parser could not determine tag type
     status_bad_pi,              // Parsing error occurred while parsing document declaration/processing instruction
     status_bad_comment,         // Parsing error occurred while parsing comment
     status_bad_cdata,           // Parsing error occurred while parsing CDATA section
     status_bad_doctype,         // Parsing error occurred while parsing document type declaration
     status_bad_pcdata,          // Parsing error occurred while parsing PCDATA section
     status_bad_start_element,   // Parsing error occurred while parsing start element tag
     status_bad_attribute,       // Parsing error occurred while parsing element attribute
     status_bad_end_element,     // Parsing error occurred while parsing end element tag
     status_end_element_mismatch // There was a mismatch of start-end tags (closing tag had incorrect name, some tag was not closed or there was an excessive closing tag)
     };

  // Parsing options

  /**
   * Minimal parsing mode (equivalent to turning all other flags off).
   * Only elements and PCDATA sections are added to the DOM tree, no text
   * conversions are performed.
   */
  static const unsigned int parse_minimal = 0x0000;

  /**
   * This flag determines if processing instructions (node_pi) are added
   * to the DOM tree. This flag is off by default.
   */
  static const unsigned int parse_pi = 0x0001;

  /**
   * This flag determines if comments (node_comment) are added to the
   * DOM tree. This flag is off by default.
   */
  static const unsigned int parse_comments = 0x0002;

  /**
   * This flag determines if CDATA sections (node_cdata) are added to
   * the DOM tree. This flag is on by default.
   */
  static const unsigned int parse_cdata = 0x0004;

  /**
   * This flag determines if plain character data (node_pcdata) that
   * consist only of whitespace are added to the DOM tree.
   * This flag is off by default; turning it on usually results in slower
   * parsing and more memory consumption.
   * NOTE: currently this option is disabled.
   */
  static const unsigned int parse_ws_pcdata = 0x0008;

  /**
   * This flag determines if character and entity references are expanded
   * during parsing. This flag is on by default.
   */
  static const unsigned int parse_escapes = 0x0010;

  /**
   * This flag determines if EOL characters are normalized
   * (converted to 0xA) during parsing. This flag is on by default.
   */
  static const unsigned int parse_eol = 0x0020;

  /**
   * This flag determines if attribute values are normalized using CDATA
   * normalization rules during parsing. This flag is on by default.
   */
  static const unsigned int parse_wconv_attribute = 0x0040;

  /**
   * This flag determines if attribute values are normalized using
   * NMTOKENS normalization rules during parsing. This flag is off by default.
   */
  static const unsigned int parse_wnorm_attribute = 0x0080;

  /**
   * This flag determines if document declaration (node_declaration) is
   *  added to the DOM tree. This flag is off by default.
   */
  static const unsigned int parse_declaration = 0x0100;

  /**
   * This flag determines if document type declaration (node_doctype) is
   * added to the DOM tree. This flag is off by default.
   */
  static const unsigned int parse_doctype = 0x0200;

  /**
   * The default parsing mode.
   * Elements, PCDATA and CDATA sections are added to the DOM tree,
   * character/reference entities are expanded, End-of-Line characters
   * are normalized, attribute values are normalized using CDATA
   * normalization rules.
   */
  static const unsigned int parse_default = parse_cdata
                                          | parse_escapes
                                          | parse_wconv_attribute
                                          | parse_eol;

  /**
   * The full parsing mode.
   * Nodes of all types are added to the DOM tree, character/reference
   * entities are expanded, End-of-Line characters are normalized,
   * attribute values are normalized using CDATA normalization rules.
   */
  static const unsigned int parse_full = parse_default
                                       | parse_pi
                                       | parse_comments
                                       | parse_declaration
                                       | parse_doctype;

public:

  /**
   * Creates new parser with the specified parsing options.
   */
  parser(unsigned int options = parse_default);

  /**
   * Parses the specified HTML string and returns document object
   * representing the HTML document tree.
   * \note
   * Capitalizes element and attribute names.
   */
  Document parse(const String& str_html);

  /**
   * @return last parse status description.
   */
  String status_description() const;

  /**
   * @return parse status string representation.
   */
  static String status_description(parse_status status);

  /**
   * Returns the parsed HTML document object.
   */
  Document get_document() const;

private:
  unsigned int options_;
  parse_status status_ = status_ok;
  Char* error_offset_ = nullptr;
  Document document_;
  Node current_node_;

  /**
   * Checks if the specified parsing option is set.
   */
  bool option_set(unsigned int opt);

  /**
   * Parse node contents, starting with exclamation mark.
   *
   * @param opts parse options. E.g. parse comments or not, etc.
   * @param endch character by which the doctype element must end.
   *  Otherwise the parse_error is thrown.
   */
  const Char* parse_exclamation(const Char* s, Char endch = '>');

  /**
   * Parses the first group of doctype primitives:
   *  1. <!-- ... -->, <? ... ?>, "...", '...'
   * This method simply advances the specified string pointer to the next
   * symbol of the end of doctype element.
   *
   * @throws parse_error on failure.
   */
  static const Char* advance_doctype_primitive(const Char* s);

  /**
   * Parses the second group of doctype:
   *  2. <![...]]>
   * This method simply advances the specified string pointer to the
   * character which is post the last char of doctype element.
   *
   * @throws parse_error on failure.
   */
  static const Char* advance_doctype_ignore(const Char* s);

  /**
   * Parses the third group of doctype:
   *  3. <!...>
   * This method simply advances the specified string pointer to the
   * character which is post the last char of doctype element.
   *
   * @param endch character by which the doctype element must end.
   *  Otherwise the parse_error is thrown.
   * @param top_level flag indicating if this method was called
   *  from top level or recursively from itself.
   */
  static const Char* advance_doctype_group(const Char* s,
                                           Char endch,
                                           bool top_level = true);
};


/*******************************************************************************
 * \defgroup TOOLS Simple helper functions.
 * Those are not needed to use this lib, but helpful.
 * doxygen: @{
 ******************************************************************************/

/**
 * Parses a html file and returns a Document shared pointer.
 * \note
 * Element and attribute names are always capitalized.
 */
Document ParseFile(std::string FileName);

/**
 * Searches a document for it's body node and returns it.
 * returns a pointer to the body node or a nullptr.
 */
Node GetBody(Document& doc);

/**
 * Searches a node for a child node by name.
 * \note
 * Name is case-insensitiv.
 * returns a pointer to the first node found or a nullptr.
 */
Node GetNode(Node& Parent, std::string Name);

/**
 * Searches a node for a child node by name and attribute-value pair.
 * \note
 * Name and AttrName are case-insensitiv.
 * returns a pointer to the first node found or a nullptr.
 */
Node GetNode(Node& Parent, std::string Name, std::string AttrName, std::string AttrValue);

/**
 * Searches a node for childs with a given name.
 * \note
 * Name is case-insensitiv.
 * returns a vector of nodes.
 */
std::vector<Node> GetNodeList(Node& Parent, std::string Name);

/**
 * Searches a node for childs with a given name and attribute-value pair.
 * \note
 * Name and AttrName are case-insensitiv.
 * returns a vector of nodes.
 */
std::vector<Node> GetNodeList(Node& Parent, std::string Name, std::string AttrName, std::string AttrValue);

/**  doxygen: end of TOOLS group. @} */


/*******************************************************************************
 * @cond INTERNAL
 *
 * HTML::tokenizer, internal class, used in parsing.
 ******************************************************************************/
class token_iterator {
public:
  typedef std::string::const_iterator const_char_iterator;
  typedef std::unordered_map<std::string, std::string> attribute_list_type;

  struct token {
     enum class token_type {
        illegal,
        doctype,
        start_tag,
        end_tag,
        comment,
        string, // Any textual data.
        end_of_file
        };
           
     token_type type;
     String value;
           
     bool has_attributes;
     attribute_list_type attributes;
     };

  token_iterator(const std::string& html);
  token* operator->();
  token_iterator& operator++();
  bool has_next() const;
  token next();
private:
  enum class tokenizer_state {
     data,
     tag_open,
     end_tag_open,
     tag_name,
     before_attribute_name,
     attribute_name,
     before_attribute_value,
     unquoted_attribute_value
     };

  const std::string html_;
  const_char_iterator it_html_;
  token current_token_;
  tokenizer_state state_;
  String curr_attribute_name_;
  String curr_attribute_value_;
  bool on_data_state();
  bool on_tag_open_state();
  bool on_end_tag_open_state();
  bool on_tag_name_state();
  bool on_before_attribute_name_state();
  bool on_attribute_name_state();
  bool on_before_attribute_value_state();
  bool on_unquoted_attribute_value_state();
  token scan_string_token();

  /**
   * @return true if EOF reached while scanning.
   */
  bool is_eof() const;

  /**
   * Creates new current token if current character is ASCII letter.
   * Sets new scanner state.
   *
   * @param type new token type.
   * @param new_state new tokenizer state.
   */
  void create_tag_token_if_curr_char_is_letter(token::token_type type,
                                               tokenizer_state new_state);
};
/* @endcond */

} // HTML.
