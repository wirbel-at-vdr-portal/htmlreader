#include <iostream>
#include <cassert>
#include <cstring>
#include <algorithm>
#include <locale>
#include <sstream>
#include <unordered_map>
#include <set>
#include <htmlreader.h>


namespace HTML {

int CurrentNumber;

class parse_error : public std::runtime_error {
public:
  static std::string format_error_msg(parser::parse_status status,
                                      const std::string& html,
                                      const char* pos,
                                      const std::string& err_msg);

  parse_error(parser::parse_status status);

  /**
   * Creates new parse error with the specified type and data
   * neccessary to locate where parse error happened.
   *
   * @param status parse error type.
   * @param str_html parsed html document string.
   * @param parse_pos pointer to html string that parses used last.
   * @param err_msg additional message to be concated to formated error.
   */
  parse_error(parser::parse_status status,
              const std::string& str_html,
              const char* parse_pos,
              const std::string& err_msg = "");

  /**
   * Returns parse error type.
   */
  parser::parse_status status() const;

private:
  parser::parse_status status_;
};


inline bool is_chartype(Char ch, enum parser::chartype_t Char) {

  /**
   * This table maps ASCII symbols with their possible types in enum chartype_t.
   */
  const unsigned char chartype_table[256] = {
      55,   0,   0,   0,   0,   0,   0,   0,      0,   12, 12,   0,   0,  62,   0,   0, // 0-15
       0,   0,   0,   0,   0,   0,   0,   0,      0,   0,   0,   0,   0,   0,   0,   0, // 16-31
      10,   0,   4,   0,   0,   0,   4,   4,      0,   0,   0,   0,   0,  96,  64,   0, // 32-47
      64,  64,  64,  64,  64,  64,  64,  64,     64,  64, 192,   0,   1,   0,  50,   0, // 48-63
       0, 192, 192, 192, 192, 192, 192, 192,    192, 192, 192, 192, 192, 192, 192, 192, // 64-79
     192, 192, 192, 192, 192, 192, 192, 192,    192, 192, 192,   0,   0,  16,   0, 192, // 80-95
       0, 192, 192, 192, 192, 192, 192, 192,    192, 192, 192, 192, 192, 192, 192, 192, // 96-111
     192, 192, 192, 192, 192, 192, 192, 192,    192, 192, 192,   0,   0,   0,   0,   0, // 112-127
     
     192, 192, 192, 192, 192, 192, 192, 192,    192, 192, 192, 192, 192, 192, 192, 192, // 128+
     192, 192, 192, 192, 192, 192, 192, 192,    192, 192, 192, 192, 192, 192, 192, 192,
     192, 192, 192, 192, 192, 192, 192, 192,    192, 192, 192, 192, 192, 192, 192, 192,
     192, 192, 192, 192, 192, 192, 192, 192,    192, 192, 192, 192, 192, 192, 192, 192,
     192, 192, 192, 192, 192, 192, 192, 192,    192, 192, 192, 192, 192, 192, 192, 192,
     192, 192, 192, 192, 192, 192, 192, 192,    192, 192, 192, 192, 192, 192, 192, 192,
     192, 192, 192, 192, 192, 192, 192, 192,    192, 192, 192, 192, 192, 192, 192, 192,
     192, 192, 192, 192, 192, 192, 192, 192,    192, 192, 192, 192, 192, 192, 192, 192
     };

  if (sizeof(Char) > 1) // HTML_WCHAR_MODE, Char is wchar_t
     return (static_cast<unsigned int>(ch) < 128 ?
        chartype_table[static_cast<unsigned int>(ch)] :
        chartype_table[128]) & (Char);
  else
     return chartype_table[static_cast<unsigned char>(ch)] & (Char);
}

// Parser utilities.

inline const Char* skip_white_spaces(const Char* str) {
  while(is_chartype(*str, parser::ct_space)) ++str;
  return str;
}


/**
 * Checks if previous sibling should be closed automatically.
 */
inline bool autoclose_prev_sibling(const std::string& tag_name, const std::string& prev_sibling_name) {
  /* End tag might be ommited for some elements, if they are followed by specific
   * elements. E.g. one might write
   *  <ul>
   *    <li>item1
   *    <li>item2
   *  </ul>
   */
  std::unordered_map<String, std::set<String> > no_end_tag_by_sibling = {
     // name , siblings
     {"LI"   , {"LI"      }},
     {"TD"   , {"TD"      }},
     {"TR"   , {"TR", "TD"}},
     {"TH"   , {"TH"      }},
     {"TBODY", {"THEAD"   }},
     {"DD"   , {"DD", "DT"}},
     {"DT"   , {"DT"      }},
     {"P"    , {"P"       }}  // a new p should close previous one.
     };

  auto it_siblings = no_end_tag_by_sibling.find(tag_name);
  if (it_siblings == std::end(no_end_tag_by_sibling))
     return false;

  auto it = it_siblings->second.find(prev_sibling_name);
  return it != std::end(it_siblings->second);
}


inline bool autoclose_last_child(const std::string& tag_name) {
  /* End tag might be ommited for some elements, if they are the last child
   * of their parent element. E.g.
   *  <ul>
   *    <li>item1
   *  </ul>
   */
  std::set<String> no_end_tag_by_child = {
     "LI",
     "P",
     "TD",
     "TR",
     "TBODY",
     "THEAD",
     "TH",
     "DD"
     };

  return no_end_tag_by_child.find(tag_name) != std::end(no_end_tag_by_child);
}


inline bool is_void_element(const std::string& tag_name) {
  std::list<String> html_void_elements = {
    "AREA",
    "BASE", "BR",
    "COL",
    "EMBED",
    "HR",
    "IMG", "INPUT",
    "KEYGEN",
    "LINK",
    "MENUITEM", "META",
    "PARAM",
    "SOURCE",
    "TRACK",
    "WBR"
    };

  auto it = std::find(std::begin(html_void_elements),
                      std::end(html_void_elements),
                      tag_name);
  return it != std::end(html_void_elements);
}



#define THROW_ERROR(err, m)  (void)m, throw parse_error(err)
#define SCANFOR(X)           { while (*s != 0 && !(X)) ++s;      }
#define SCANWHILE(X)         { while ((X)) ++s;                  }
#define CHECK_ERROR(err, m)  { if (*s == 0) THROW_ERROR(err, m); }
// Utility macro for last character handling
#define ENDSWITH(c, e) ((c) == (e) || ((c) == 0 && endch == (e)))



const Char* parser::advance_doctype_primitive(const Char* s) {
  // Quoted string.
  if (*s == '"' || *s == '\'') {
     Char ch = *s++;
     SCANFOR(*s == ch);
     if (!*s) THROW_ERROR(status_bad_doctype, s);
     ++s;
     }
  // <? ... ?>
  else if (s[0] == '<' && s[1] == '?') {
     s += 2;
     // no need for ENDSWITH because ?> can't terminate proper doctype
     SCANFOR(s[0] == '?' && s[1] == '>');
     if (!*s) THROW_ERROR(status_bad_doctype, s);
     s += 2;
     }
  // <-- ... -->
  else if (s[0] == '<' && s[1] == '!' && s[2] == '-' && s[3] == '-') {
     s += 4;
     // no need for ENDSWITH because --> can't terminate proper doctype
     SCANFOR(s[0] == '-' && s[1] == '-' && s[2] == '>');
     if (!*s) THROW_ERROR(status_bad_doctype, s);
     s += 4;
     }
  else {
     THROW_ERROR(status_bad_doctype, s);
     }

  return s;
}


const Char* parser::advance_doctype_ignore(const Char* s) {
  assert(s[0] == '<' && s[1] == '!' && s[2] == '[');
  ++s;

  while(*s) {
     if (s[0] == '<' && s[1] == '!' && s[2] == '[') {
        // Nested ignore section.
        s = advance_doctype_ignore(s);
        }
     else if (s[0] == ']' && s[1] == ']' && s[2] == '>') {
        // Ignore section end.
        return s + 3;
        }
     else {
        ++s;
        }
     }

  THROW_ERROR(status_bad_doctype, s);
  return s;
}


const Char* parser::advance_doctype_group(const Char* s, Char endch, bool top_level) {
  assert(s[0] == '<' && s[1] == '!');
  ++s;

  while(*s) {
     if (s[0] == '<' && s[1] == '!') {
        if (s[2] == '[') {
           // Ignore.
           s = advance_doctype_ignore(s);
           }
        else {
           // Some control group.
           s = advance_doctype_group(s, endch, false);
           }
        }
     else if (s[0] == '<' || s[0] == '"' || s[0] == '\'') {
        // unknown tag (forbidden), or some primitive group
        s = advance_doctype_primitive(s);
        }
     else if (*s == '>') {
        return ++s;
        }
     else {
        ++s;
        }
     }

  if (!top_level || endch != '>') {
     THROW_ERROR(status_bad_doctype, s);
     }

  return s;
}


parser::parser(unsigned int options) :
  options_(options),
  document_(document::create()),
  current_node_(document_),
  debug(false) { }


const Char* parser::parse_exclamation(const Char* s, Char endch) {
  // Skip '<!'.
  s += 2;

  // '<!-...' - comment.
  if (*s == '-') {
     ++s;
     if (*s != '-') {
        THROW_ERROR(status_bad_comment, s);
        }

     ++s;
     const Char* comment_start = s;

     // Scan for terminating '-->'.
     SCANFOR(s[0] == '-' && s[1] == '-' && ENDSWITH(s[2], '>'));
     CHECK_ERROR(status_bad_comment, s);

     if (this->option_set(parse_comments)) {
        size_t comment_len = (s - 1) - comment_start + 1;
        String comment(comment_start, comment_len);

        auto comment_node = node::create(node::node_comment);
        comment_node->value(comment);

        this->current_node_->append_child(comment_node);
        }

     // Step over the '\0->'.
     s += (s[2] == '>' ? 3 : 2);
     }
  // '<![CDATA[...'
  else if (*s == '[') {
     if (!(*++s=='C' && *++s=='D' && *++s=='A' && *++s=='T' && *++s=='A' && *++s == '[')) {
        THROW_ERROR(status_bad_cdata, s);
        }

     ++s;
     const Char* cdata_start = s;

     SCANFOR(s[0] == ']' && s[1] == ']' && ENDSWITH(s[2], '>'));
     CHECK_ERROR(status_bad_cdata, s);

     if (this->option_set(parse_cdata)) {
        size_t cdata_len = s - cdata_start + 1;
        String cdata(cdata_start, cdata_len);

        auto node = node::create(node::node_cdata);
        node->value(cdata);
        this->current_node_->append_child(node);
        }

     ++s;
     s += (s[1] == '>' ? 2 : 1); // Step over the last ']>'.
     }
  // <!DOCTYPE
  else if (s[0] == 'D' && s[1] == 'O' && s[2] == 'C' && s[3] == 'T' && s[4] == 'Y' && s[5] == 'P' && ENDSWITH(s[6], 'E')) {
     s -= 2;

     const Char* doctype_start = s + 9;

     s = advance_doctype_group(s, endch);

     if (this->option_set(parse_doctype)) {
        while (is_chartype(*doctype_start, parser::ct_space)) {
        ++doctype_start;
        }

        assert(s[-1] == '>');
        size_t doctype_len = (s - 2) - doctype_start + 1;
        String doctype(doctype_start, doctype_len);

        auto node = node::create(node::node_doctype);
        node->value(doctype);
        this->current_node_->append_child(node);
        }
     }
  else if (*s == 0 && endch == '-')
     THROW_ERROR(status_bad_comment, s);
  else if (*s == 0 && endch == '[')
     THROW_ERROR(status_bad_cdata, s);
  else
     THROW_ERROR(status_unrecognized_tag, s);

  return s;
}


inline void str_toupper(String& str) {
  std::locale loc;
  for(auto it = std::begin(str); it != std::end(str); ++it)
     *it = std::toupper(*it, loc);
}


Node find_parent_node_for_new_tag(Node current_node, const String& new_tag_name) {
  auto new_tag_parent = current_node;

  auto parent = current_node->parent();
  if (parent && autoclose_prev_sibling(new_tag_name, current_node->name())) {
     while(parent->parent() && autoclose_prev_sibling(new_tag_name, parent->name())) {
        parent = parent->parent();
        }
     new_tag_parent = parent;
     }

  return new_tag_parent;
}


Document parser::parse(const String& str_html) {
  this->status_ = status_ok;
  CurrentNumber = 1;

  if (str_html.size() == 0) {
     if (debug)
        std::cerr << __PRETTY_FUNCTION__ << ": cannot parse empty string." << std::endl;
     return this->document_;
     }

  const Char* s = str_html.c_str();

  // Flag indicating if last parsed tag is void html element.
  bool last_element_void = false;

  auto on_tag_start = [&](const std::string& tag_name) {
     if (debug) std::cerr << "<" << tag_name << " (" << std::to_string(CurrentNumber) << ")";
     if (last_element_void) {
        this->current_node_ = this->current_node_->parent();
        last_element_void = false;
        }

     auto node = node::create(node::node_element);
     node->name(tag_name);
     node->number(CurrentNumber++);

     auto new_tag_parent = find_parent_node_for_new_tag(this->current_node_, tag_name);
     new_tag_parent->append_child(node);
     if (debug) std::cerr << ": parent = " << new_tag_parent->name()
                          << " (" << std::to_string(new_tag_parent->number()) << ")" << std::endl;

     this->current_node_ = node;
     };

  auto on_closing_tag = [&](const std::string& tag_name) {
     if (debug) std::cerr << "</" << tag_name << ">;  current node: " << this->current_node_->name() << " (" << std::to_string(this->current_node_->number()) << ")" << std::endl;
     if (tag_name != this->current_node_->name() &&
         (autoclose_last_child(this->current_node_->name()) || last_element_void)) {
        if (debug) {
           std::string ac = autoclose_last_child(this->current_node_->name()) ? "true" : "false";
           std::string ev = last_element_void ? "true" : "false";
           std::cerr << "autoclose = " << ac << " last_element_void = " << ev << std::endl;
           }
        if (last_element_void) {
           this->current_node_ = this->current_node_->parent();
           if (debug)
              std::cerr << "last_element_void; current node = " << this->current_node_->name()
                        << "(" << std::to_string(this->current_node_->number()) << ")" << std::endl;
           }
        else
           while(autoclose_last_child(this->current_node_->name())) {
              this->current_node_ = this->current_node_->parent();
              // CHECK THE NEXT IF()..
              if (tag_name == this->current_node_->name()) {
                 if (debug)
                    std::cerr << "stop autoclose on " << this->current_node_->name()
                              << "(" << std::to_string(this->current_node_->number()) << ") "
                              << "parent = " << this->current_node_->parent()->number() << std::endl;
                 break;
                 }
              if (debug)
                 std::cerr << "autoclose_last_child; current node = " << this->current_node_->name()
                           << "(" << std::to_string(this->current_node_->number()) << ")" << std::endl;
              }

        last_element_void = false;
        }

     const String& expected_name = this->current_node_->name();
     if (expected_name != tag_name) {
        //if (debug) {
        // std::string text(s, 160);
        // //size_t p = text.find("\n");
        // std::cerr << "line='" << /*text.substr(0,p-1)*/ text << "'" << std::endl;
        // }
        std::string err_msg = "Expected: '"
                            + expected_name
                            + "', found: '" + tag_name + "'";
        throw parse_error(status_end_element_mismatch, str_html, s, err_msg);
        }

     if (this->current_node_->parent()) {
        this->current_node_ = this->current_node_->parent();
        last_element_void = false;
        }
     };

  auto on_pcdata = [&](const std::string& pcdata) {
     if (debug and pcdata.find_first_not_of(" \t\n") != std::string::npos) std::cerr << "pcdata: '" << pcdata << "'" << std::endl;
     if (last_element_void) {
        this->current_node_ = this->current_node_->parent();
        last_element_void = false;
        }
     auto node = node::create(node::node_cdata);
     node->value(pcdata);
     this->current_node_->append_child(node);
     };

  auto on_attribute = [&](const std::string& attr_name, const std::string& attr_val) {
     if (debug) std::cerr << "  " << attr_name << " = '" << attr_val << "'" << std::endl;
     auto attr = attribute::create(attr_name, attr_val);
     this->current_node_->append_attribute(attr);
     };

  auto on_script = [&](const String& script_value) {
     if (debug) std::cerr << "script: '" << script_value << "'" << std::endl;
     auto node = node::create(node::node_cdata);
     node->value(script_value);
     this->current_node_->append_child(node);
     };

  auto parse_pcdata = [&]() {
     //if (debug) std::cerr << "parse_pcdata" << std::endl;
     const Char* pcdata_start = s;
     while(!is_chartype(*s, parser::ct_parse_pcdata)) ++s;
     size_t pcdata_len = (s - 1) - pcdata_start + 1;
     String pcdata = String(pcdata_start, pcdata_len);
     on_pcdata(pcdata);
     };

  auto parse_script = [&]() {
     if (debug) std::cerr << "parse_script" << std::endl;
     const Char* script_value_start = s;
     const Char* script_value_end = s;

     while(*s != '\0') {
        if (strncmp(s, "</script>", 9) == 0) {
           script_value_end = s - 1;
           break;
           }
        else
           ++s;
        }

     size_t script_value_len = script_value_end - script_value_start + 1;
     String script_value = String(script_value_start, script_value_len);
     on_script(script_value);
     };

  auto on_attribute_name_state = [&]() {
     //if (debug) std::cerr << "on_attribute_name_state" << std::endl;
     const Char* attr_name_start = s;

     SCANWHILE(is_chartype(*s, parser::ct_symbol));
     if (*s == '\0')
        throw parse_error(status_bad_attribute, str_html, s);

     size_t attr_name_len = (s - 1) - attr_name_start + 1;
     String attr_name = String(attr_name_start, attr_name_len);
     str_toupper(attr_name);

     s = skip_white_spaces(s);
     if (*s == '\0')
        throw parse_error(status_bad_attribute, str_html, s);

     String attr_val;
     // Attribute with value.
     if (*s == '=') {
        ++s;
        s = skip_white_spaces(s);

        Char quote_symbol = 0;
        if (*s == '"' || *s == '\'') {
           quote_symbol = *s;
           ++s;
           }

        const Char* attr_val_start = s;
        if (quote_symbol) {
           while (*s && *s != quote_symbol) ++s;

          if (*s != quote_symbol) {
             throw parse_error(status_bad_attribute, str_html, s,
                               "Bad attribute value closing symbol.");
             }
          }
        else {
           while (!is_chartype(*s, parser::ct_parse_attr)) ++s;
           }

        size_t attr_val_len = (s - 1) - attr_val_start + 1;
        attr_val = String(attr_val_start, attr_val_len);

        if (quote_symbol) {
           // Step over attribute value stop symbol.
           ++s;
           }
        else {
           s = skip_white_spaces(s);
           }
        }
     else {
        // Attribute has no value.
        s = skip_white_spaces(s);
        if (*s == '\0') {
           throw parse_error(status_bad_attribute, str_html, s);
           }
        }

    on_attribute(attr_name, attr_val);
  };

  auto on_self_closing_start_tag_state = [&]() {
     if (debug) std::cerr << "on_self_closing_start_tag_state" << std::endl;
     ++s;

     if (*s != '>') {
        throw parse_error(status_bad_start_element, str_html, s);
        }
     else {
        last_element_void = true;
        }
     };

  auto on_tag_open_state = [&]() {
     //if (debug) std::cerr << "on_tag_open_state" << std::endl;
     ++s;
     //if (debug) std::cerr << __FUNCTION__ << ": '" << *s << "'" << std::endl;
     // Check if the current character is a tag start symbol.
     if (is_chartype(*s, parser::ct_start_symbol)) {
        const Char* tag_name_start = s;

        // Scan while the current character is a symbol belonging
        // to the set of symbols acceptable within a tag. In other
        // words, scan until the termination symbol is discovered.
        SCANWHILE(is_chartype(*s, parser::ct_symbol));

        size_t tag_name_len = (s - 1) - tag_name_start + 1;
        String tag_name = String(tag_name_start, tag_name_len);
        str_toupper(tag_name);

        on_tag_start(tag_name);

        // End of tag.
        if (*s == '>') {
           last_element_void = is_void_element(this->current_node_->name());
           }
        else if (is_chartype(*s, parser::ct_space)) {
          while(true) {
             s = skip_white_spaces(s);

             // Attribute start.
             if (is_chartype(*s, parser::ct_start_symbol))
                on_attribute_name_state();

             // Void element end.
             else if (*s == '/') {
                on_self_closing_start_tag_state();
                break;
                }
             // Tag end, also might be void element.
             else if (*s == '>') {
                last_element_void = is_void_element(this->current_node_->name());
                break;
                }
             else
                throw parse_error(status_bad_start_element, str_html, s);
             } // while
           }
        // Void HTML element.
        else if (*s == '/')
           on_self_closing_start_tag_state();
        else
           throw parse_error(status_bad_start_element, str_html, s);
        ++s;
        }
     // Closing tag, e.g. </hmtl>
     else if (*s == '/') {
        ++s;

        const Char* tag_name_start = s;
        while(is_chartype(*s, parser::ct_symbol)) ++s;

        size_t tag_name_len = (s - 1) - tag_name_start + 1;
        String tag_name = String(tag_name_start, tag_name_len);
        str_toupper(tag_name);

        on_closing_tag(tag_name);

        s = skip_white_spaces(s);
        if (*s != '>')
           THROW_ERROR(status_bad_end_element, "");
        ++s;
        }
     // Comment: <!-- ...
     else if (*s == '!')
        s = parse_exclamation(s - 1);
     else
        throw parse_error(status_unrecognized_tag, str_html, s);
     };

  this->current_node_ = this->document_;

  // Parse while the current character is not '\0'.
  while(*s != '\0') {
     //if (debug) std::cerr << __FUNCTION__ << ":" << __LINE__ << " '" << *s << "'" << std::endl;
     // Check if the current character is the start tag character
     if (*s == '<')
        on_tag_open_state();
     else {
        if (this->current_node_->name() == "SCRIPT")
           parse_script();
        else
           parse_pcdata();
        }
     }

  return this->document_;
}


String parser::status_description() const {
  return parser::status_description(this->status_);
}


String parser::status_description(parse_status status) {
  switch(status) {
     case status_ok:                   return "No error.";
     case status_file_not_found:       return "File was not found.";
     case status_io_error:             return "Error reading from file/stream.";
     case status_out_of_memory:        return "Could not allocate memory.";
     case status_internal_error:       return "Internal error occurred.";
     case status_unrecognized_tag:     return "Could not determine tag type.";
     case status_bad_pi:               return "Error parsing document declaration/processing instruction.";
     case status_bad_comment:          return "Error parsing comment.";
     case status_bad_cdata:            return "Error parsing CDATA section.";
     case status_bad_doctype:          return "Error parsing document type declaration.";
     case status_bad_pcdata:           return "Error parsing PCDATA section.";
     case status_bad_start_element:    return "Error parsing start element tag.";
     case status_bad_attribute:        return "Error parsing element attribute.";
     case status_bad_end_element:      return "Error parsing end element tag.";
     case status_end_element_mismatch: return "Start-end tags mismatch.";
     default:                          return "Unknown error.";
     }
}


Document parser::get_document() const {
  return this->document_;
}


// parse_error

parse_error::parse_error(parser::parse_status status) :
  std::runtime_error(parser::status_description(status)), status_(status) { }


parse_error::parse_error(parser::parse_status status,
                         const std::string& str_html,
                         const char* parse_pos,
                         const std::string& err_msg) :
  std::runtime_error(parse_error::format_error_msg(status,
                                                   str_html,
                                                   parse_pos,
                                                   err_msg)),
                     status_(status) { }

parser::parse_status parse_error::status() const {
  return this->status_;
}


std::string parse_error::format_error_msg(parser::parse_status status,
                                          const std::string& html,
                                          const char* pos,
                                          const std::string& err_msg) {
  size_t line_nr = 0;

  auto find_newline = [&](const char* start) {
     return std::find_if(start, pos, [](char symb) { return symb == '\n'; });
     };

  const char* str_html = html.c_str();
  auto it = find_newline(str_html);
  auto last_newline = str_html;
  while(it < pos) {
     last_newline = it;
     ++line_nr;
     it = find_newline(it + 1);
     }

  size_t row_nr = pos - last_newline;

  size_t chars_to_print = str_html + html.size() - pos >= 20 ? 20 : str_html + html.size() - pos;
  std::stringstream ss;
  ss << parser::status_description(status)
     << " Line: "
     << line_nr
     << ", column: "
     << row_nr
     << ": '"
     << std::string(pos, chars_to_print)
     << "...'. "
     << err_msg;
  return ss.str();
}



// Private methods.

bool parser::option_set(unsigned int opt) {
  return this->options_ & opt;
}

} // namespace HTML
