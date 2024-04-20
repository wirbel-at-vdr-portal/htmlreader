#include <htmlreader.h>


namespace HTML {

attribute::attribute(const String& name, const String& value) :
  node(node_attribute), name_(name), value_(value) { }


Attribute attribute::create(const String& name, const String& value) {
  return Attribute(new attribute(name, value));
}


bool attribute::operator==(const attribute& attr) const {
  return this->name_ == attr.name_ && this->value_ == attr.value_;
}


bool attribute::operator!=(const attribute& attr) const {
  return !(*this == attr);
}


attribute& attribute::operator=(const String& attr_val) {
  this->value_ = attr_val;
  return *this;
}


String attribute::name() const {
  return this->name_;
}


String attribute::value() const {
  return this->value_;
}


void attribute::value(const String& attr_val) {
  this->value_ = attr_val;
}

} // namespace HTML
