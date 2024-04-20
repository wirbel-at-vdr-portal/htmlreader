#include <htmlreader.h>


namespace HTML {

Document document::create() {
  return Document(new document());
}

document::document() : node(node_document) { }


class links_walker : public node_walker {
public:
  links_walker(std::vector<Node>& links) : links_(links) { }

  bool for_each(Node node) override {
     if (node->name() == "A" || node->name() == "AREA")
        this->links_.push_back(node);
     return true;
     }
private:
  std::vector<Node>& links_;
};


std::vector<Node> document::links() const {
  std::vector<Node> result;

  links_walker html_walker(result);
  const_cast<document*>(this)->traverse(html_walker);
  return result;
}


Node document::get_element_by_id(const String& id) const {
  return this->find_node(
                         [&](const Node& node) {
                             auto attr = node->get_attribute("ID");
                             return attr ? attr->value() == id : false;
                             }
                        );
}


class tags_walker : public node_walker {
public:
  std::vector<Node> tag_elements;

  tags_walker(const String& tag_name) : tag_name_(tag_name) { }

  bool for_each(Node node) override {
     if (node->name() == this->tag_name_)
        this->tag_elements.push_back(node);
     return true;
     }

private:
  const String tag_name_;
};

std::vector<Node> document::get_elements_by_tag_name(const String& tag_name) const {
  tags_walker html_walker(tag_name);
  const_cast<document*>(this)->traverse(html_walker);
  return html_walker.tag_elements;
}

} // namespace HTML
