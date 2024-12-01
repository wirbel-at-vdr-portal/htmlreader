#include <iostream>
#include <new>
#include <algorithm>
#include <iterator>
#include <sstream>
#include <htmlreader.h>


namespace HTML {

inline bool allow_insert_child(node::node_type parent, node::node_type child) {
  if (parent != node::node_document && parent != node::node_element)
     return false;

  if (child == node::node_document || child == node::node_null)
     return false;

  if (parent != node::node_document && (child == node::node_declaration || child == node::node_doctype))
     return false;

  return true;
}


node::node(node_type type) : type_(type), Number(0), SubNumber(0) { }


Node node::create(node_type type) {
  return Node(new node(type));
}


node::node_type node::type() const {
  return this->type_;
}


String node::name() const {
  return this->name_;
}


void node::name(const String& name) {
  size_t p = name.find_first_of(" \t");
  if (p == std::string::npos)
     this->name_ = name;
  else {
     std::cerr << __PRETTY_FUNCTION__ << ": using '" << name.substr(0, p-1)
               << "' instead of '" << name << "'" << std::endl;
     this->name_ = name.substr(0, p-1);
     }
}


String node::value() const {
  return this->value_;
}


void node::value(const String& value) {
  switch(type()) {
     case node_pi:
     case node_cdata:
     case node_pcdata:
     case node_comment:
     case node_doctype:
        this->value_ = value;
        break;
     default:
        return;
     }
}


String node::text_content() const {
  if (this->children_.size() == 0)
     return "";

  std::string text;
  auto tree_walker = make_node_walker(
                        [&](Node node, std::size_t) {
                           text += node->value();
                           return true;
                           }
                        );
  (const_cast<node*>(this))->traverse(*tree_walker);

  return text;
}


Attribute node::first_attribute() const {
  return this->attributes_.empty() ? nullptr : this->attributes_.front();
}


Attribute node::last_attribute() const {
  return this->attributes_.empty() ? nullptr : this->attributes_.back();
}


Attribute node::get_attribute(const String& name) const {
  auto it_attr = std::find_if(std::begin(this->attributes_),
                              std::end(this->attributes_),
                              [&](const Attribute& attr) {
                                 return attr->name() == name;
                                 }
                             );

  return it_attr == this->attributes_.cend() ? nullptr : *it_attr;
}


Attribute node::append_attribute(const String& name, const String& value) {
  auto attr = attribute::create(name, value);
  this->attributes_.push_back(attr);
  return attr;
}


Attribute node::append_attribute(Attribute attr) {
  this->attributes_.push_back(attr);
  return attr;
}


Attribute node::prepend_attribute(const String& name, const String& value) {
  auto attr = attribute::create(name, value);
  this->attributes_.push_front(attr);
  return attr;
}


Attribute node::prepend_attribute(Attribute attr) {
  this->attributes_.push_front(attr);
  return attr;
}


bool node::remove_attribute(const String& name) {
  bool result = false;
  this->attributes_.remove_if(
                              [&](const Attribute& attr) {
                                 result = attr->name() == name;
                                 return result;
                                 }
                             );
  return result;
}

template <typename T> Attribute node::find_attribute(T pred) const {
  auto it_attr = std::find_if(std::begin(this->attributes_),
                              std::end(this->attributes_),
                              pred);
  return it_attr == std::end(this->attributes_) ? nullptr : *it_attr;
}

Node node::first_child() const {
  return this->children_.empty() ? nullptr : this->children_.front();
}


Node node::last_child() const {
  return this->children_.empty() ? nullptr : this->children_.back();
}


Node node::child(const String& name) const {
  auto it_child = std::find_if(std::begin(this->children_),
                               std::end(this->children_),
                               [&](const Node& child) {
                                   return child->name() == name;
                                  }
                              );
  return it_child == std::end(this->children_) ? nullptr : *it_child;
}


Node node::next_sibling() const {
  auto it_next_node = this->parent_it_;
  auto parent = this->parent_.lock();

  if (parent && ++it_next_node != std::end(parent->children_))
     return *it_next_node;

  return nullptr;
}


Node node::next_sibling(const String& name) const {
  auto parent = this->parent_.lock();
  if (!parent)
     return nullptr;

  auto it_next_sibling = this->parent_it_;
  ++it_next_sibling;

  auto result = std::find_if(it_next_sibling,
                             std::end(parent->children_),
                             [&](const Node& child) {
                                return child->name() == name;
                                }
                            );
  return result == std::end(parent->children_) ? nullptr : *result;
}


Node node::previous_sibling() const {
  auto parent = this->parent_.lock();

  if (parent && this->parent_it_ != std::begin(parent->children_))
     return *(--node::iterator(this->parent_it_));

  return nullptr;
}


Node node::previous_sibling(const String& name) const {
  auto parent = this->parent_.lock();
  if (!parent)
     return nullptr;

  if (this->parent_it_ == std::begin(parent->children_))
     return nullptr;

  auto result = std::end(parent->children_);
  for(auto it_prev_sibling = this->parent_it_; it_prev_sibling != std::begin(parent->children_); ) {
     --it_prev_sibling;
     if ((*it_prev_sibling)->name() == name)
        result = it_prev_sibling;
     }

  if (result != std::end(parent->children_))
     return *result;

  return nullptr;
}


Node node::parent() const {
  return this->parent_.lock();
}


Node node::root() const {
  Node result;

  auto parent = this->parent_.lock();
  while(parent) {
     result = parent;
     parent = parent->parent_.lock();
     }

  return result;
}


String node::child_value() const {
  auto it_child = std::find_if(std::begin(this->children_),
                               std::end(this->children_),
                               [&](const Node& child) {
                                   return child->type() == node_pcdata || child->type() == node_cdata;
                                  }
                              );

  return it_child == std::end(this->children_) ? "" : (*it_child)->value();
}


String node::child_value(const String& name) const {
  auto child = this->child(name);
  return child ? child->child_value() : "";
}


void node::append_child(Node aNode) {
  aNode->parent_ = this->shared_from_this();
  this->children_.push_back(aNode);
  aNode->parent_it_ = --std::end(this->children_);
}


void node::prepend_child(Node aNode) {
  aNode->parent_ = this->shared_from_this();
  this->children_.push_front(aNode);
  aNode->parent_it_ = std::begin(this->children_);
}


bool node::remove_child(const String& name) {
  bool result = false;

  this->children_.remove_if([&](const Node& child) {
                               result = true;
                               return child->name() == name;
                               }
                           );
  return result;
}


const std::list<Node>& node::child_nodes() const {
  return this->children_;
}


template <typename T> Node node::find_child(T pred) const {
  auto it_node = find_if(this->children_.cbegin(),
                         this->children_.cend(),
                         pred);
  return it_node != std::end(this->children_) ? *it_node : nullptr;
}


std::list<Node> node::find_nodes(std::function<bool (Node)> predicate) const {
  std::list<Node> found_nodes;

  const_cast<node*>(this)->traverse([&](Node node) {
                                       if (predicate(node))
                                          found_nodes.push_back(node);
                                       bool keep_traversing = true;
                                       return keep_traversing;
                                       }
                                   );
  return found_nodes;
}


Node node::find_child_by_attribute(const String& tag,
                                                    const String& attr_name,
                                                    const String& attr_value) const {
  auto it_child = std::find_if(std::begin(this->children_),
                               std::end(this->children_),
                               [&](const Node& child) {
                                  if (child->name() != tag)
                                     return false;

                                  Attribute attr = child->get_attribute(attr_name);
                                  return attr && attr->value() == attr_value;
                                  }
                               );

  return it_child == std::end(this->children_) ? nullptr : *it_child;
}


Node node::find_child_by_attribute(const String& attr_name,
                                                    const String& attr_value) const {
  auto it_child = std::find_if(std::begin(this->children_),
                               std::end(this->children_),
                               [&](const Node& child) {
                                  Attribute attr = child->get_attribute(attr_name);
                                  return attr && attr->value() == attr_value;
                                  }
                              );
  return it_child == std::end(this->children_) ? nullptr : *it_child;
}


String node::path(Char delimiter) const {
  String result = this->name();

  auto curr_node = this->parent();
  while(curr_node) {
     result = curr_node->name() + delimiter + result;
     curr_node = curr_node->parent();
     }

  return result;
}


Node node::first_element_by_path(const String& path,
                                                  Char delimiter) const {
  (void) path;
  (void) delimiter;
  throw std::runtime_error("Not implemented.");
  return nullptr;
}


bool node::traverse(node_walker& walker) {
  if (!walker.begin(this->shared_from_this()))
     return false;

  walker.depth_ = 0;
  Node child = this->first_child();
  std::string childname("null");
  if (child) childname = child->name();
  std::cout << __PRETTY_FUNCTION__ << ":" << __LINE__ << ": child=" << childname << std::endl;

  if (child) {
     ++walker.depth_;

     std::cout << __PRETTY_FUNCTION__ << ":" << __LINE__ << ": child->first_child()=" << child->first_child() << std::endl;
     std::cout << __PRETTY_FUNCTION__ << ":" << __LINE__ << ": child->next_sibling()=" << child->next_sibling() << std::endl;

     do {
        if (!walker.for_each(child))
           return false;

        if (child->first_child()) {
           ++walker.depth_;
           child = child->first_child();
           }
        else if (child->next_sibling()) {
           child = child->next_sibling();
           }
        else {
           // Borland C++ workaround
           while(!child->next_sibling() && walker.depth_ > 0 && child->parent()) {
              --walker.depth_;
              child = child->parent();
              }
           if (walker.depth_ > 0)
              child = child->next_sibling();
           }
        }
     while(child && walker.depth_ > 0);
     }

  return walker.end(this->shared_from_this());
}


bool node::traverse(std::function<bool (Node)> predicate,
                    std::size_t depth) {
  if (depth > 0) {
     bool proceed = predicate(this->shared_from_this());
     if (!proceed)
        return false;
     }

  for(auto child:this->children_) {
     bool proceed = child->traverse(predicate, depth + 1);
     if (!proceed)
        return false;
     }

  return true;
}


node::iterator node::begin() {
  return std::begin(this->children_);
}


node::iterator node::end() {
  return std::end(this->children_);
}


node::attribute_iterator node::attributes_begin() {
  return std::begin(this->attributes_);
}


node::attribute_iterator node::attributes_end() {
  return std::end(this->attributes_);
}


inline String make_tabs(size_t tab_count) {
  String tabs;
  for(size_t i=1; i<=tab_count; ++i)
     tabs += '\t';
  return tabs;
}


inline String make_tag_name(std::size_t indentation,
                            String tag_name,
                            String tag_prefix) {
  if (tag_name == "")
     return "";

  return make_tabs(indentation) + tag_prefix + tag_name + ">\n";
}


inline String make_start_tag_name(std::size_t indentation,
                                  String tag_name) {
  return make_tag_name(indentation, tag_name, "<");
}


inline String make_end_tag_name(std::size_t indentation,
                                String tag_name) {
  return make_tag_name(indentation, tag_name, "</");
}


String node::to_string(std::size_t indentation) const {
  String str_html = make_start_tag_name(indentation, this->name());

  for(auto node:this->children_)
     str_html += node->to_string(indentation + 1);

  return str_html + make_end_tag_name(indentation, this->name());
}


node_walker::node_walker(): depth_(0) { }


node_walker::~node_walker() { }


int node_walker::depth() const {
  return this->depth_;
}


bool node_walker::begin(Node) {
  return true;
}


bool node_walker::end(Node) {
  return true;
}


class for_each_walker : public node_walker {
public:
  for_each_walker(node_walker_callback for_each): for_each_(for_each) {}
  virtual bool for_each(Node node) override {
     return this->for_each_(node, this->depth());
     }
private:
  node_walker_callback for_each_;
};

NodeWalker make_node_walker(node_walker_callback for_each) {
  return std::make_shared<for_each_walker>(for_each);
}

} // namespace HTML
