#include <map>
#include <iostream>
#include <htmlreader.h>


namespace HTML {

const Char tag_close_char = '>';

inline bool is_chartype(Char ch, enum parser::chartype_t Char) {
  /**
   * This table maps ASCII symbols with their possible types in enum parser::chartype_t.
   */
  const unsigned char chartype_table[256] = {
      55,   0,   0,   0,   0,   0,   0,   0,      0,  12,  12,   0,   0,  62,   0,   0, // 0-15
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

  return chartype_table[static_cast<unsigned char>(ch)] & (Char);
}

token_iterator::token_iterator(const std::string& html) :
  html_(html), it_html_(this->html_.cbegin() - 1),
  current_token_{token::token_type::illegal, "", false, attribute_list_type{}},
  state_(tokenizer_state::data)
{
  this->current_token_ = this->next();
}


token_iterator::token* token_iterator::operator->() {
  return &this->current_token_;
}


token_iterator& token_iterator::operator++() {
  this->current_token_ = this->next();
  return *this;
}


bool token_iterator::has_next() const {
  return this->it_html_ == this->html_.cend();
}


void token_iterator::create_tag_token_if_curr_char_is_letter(token::token_type type,
                                                             tokenizer_state new_state) {
  if (is_chartype(*this->it_html_, parser::ct_start_symbol)) {
     this->state_ = new_state;

     std::string tag_name;
     tag_name = *this->it_html_;
     this->current_token_ = token_iterator::token{type, tag_name, false, attribute_list_type{}};
     }
}


bool token_iterator::on_data_state() {
  bool token_emitted = false;
  const Char tag_open_char = '<';

  ++this->it_html_;

  // Check if the current character is the start tag character
  if (*this->it_html_ == tag_open_char)
     this->state_ = tokenizer_state::tag_open;
  else
     this->create_tag_token_if_curr_char_is_letter(token_iterator::token::token_type::start_tag, tokenizer_state::tag_name);

  return token_emitted;
}


bool token_iterator::on_tag_open_state() {
  bool token_emitted = false;
  const Char solidus_char = '/';

  ++this->it_html_;

  if (*this->it_html_ == solidus_char)
     this->state_ = tokenizer_state::end_tag_open;
  else
     this->create_tag_token_if_curr_char_is_letter(token_iterator::token::token_type::start_tag, tokenizer_state::tag_name);

  return token_emitted;
}


bool token_iterator::on_end_tag_open_state() {
  ++this->it_html_;

  this->create_tag_token_if_curr_char_is_letter(token_iterator::token::token_type::end_tag, tokenizer_state::tag_name);

  return false;
}


bool token_iterator::on_tag_name_state() {
  bool token_emitted = false;

  ++this->it_html_;

  while(is_chartype(*this->it_html_, parser::ct_start_symbol)) {
     this->current_token_.value += *this->it_html_;
     ++this->it_html_;
     }

  if (*this->it_html_ == tag_close_char) {
     token_emitted = true;
     this->state_ = tokenizer_state::data;
     }
  else if (is_chartype(*this->it_html_, parser::ct_space))
     this->state_ = tokenizer_state::before_attribute_name;

  return token_emitted;
}


bool token_iterator::is_eof() const {
  return this->it_html_ == this->html_.end();
}


bool token_iterator::on_before_attribute_name_state() {
  ++this->it_html_;

  if (this->is_eof())
     throw std::runtime_error("Unexpected EOF.");

  while (is_chartype(*this->it_html_, parser::ct_space))
     ++this->it_html_;

  bool token_emitted = false;
  if (*this->it_html_ == tag_close_char) {
     this->state_ = tokenizer_state::data;
     token_emitted = true;
     }
  else if (is_chartype(*this->it_html_, parser::ct_start_symbol)) {
     this->current_token_.has_attributes = true;
     this->curr_attribute_name_ = *this->it_html_;
     this->state_ = tokenizer_state::attribute_name;
     }

  return token_emitted;
}


bool token_iterator::on_attribute_name_state() {
  const Char equals_sign_char = '=';
  ++this->it_html_;

  if (is_chartype(*this->it_html_, parser::ct_start_symbol))
     this->curr_attribute_name_ += *this->it_html_;
  else if (*this->it_html_ == equals_sign_char)
     this->state_ = tokenizer_state::before_attribute_value;

  return false;
}


bool token_iterator::on_before_attribute_value_state() {
  ++this->it_html_;

  if (is_chartype(*this->it_html_, parser::ct_start_symbol)) {
     this->curr_attribute_value_ = *this->it_html_;
     this->state_ = tokenizer_state::unquoted_attribute_value;
     }

  return false;
}


bool token_iterator::on_unquoted_attribute_value_state() {
  ++this->it_html_;

  bool token_emitted = false;

  if (is_chartype(*this->it_html_, parser::ct_start_symbol))
     this->curr_attribute_value_ += *this->it_html_;
  else if (*this->it_html_ == tag_close_char) {
     this->current_token_.attributes[this->curr_attribute_name_] = this->curr_attribute_value_;
     token_emitted = true;
     }

  return token_emitted;
}


token_iterator::token token_iterator::next() {
  std::map<tokenizer_state, std::function<bool()>> state_handlers = {
     {tokenizer_state::data                    , std::bind(&token_iterator::on_data_state                    , this)},
     {tokenizer_state::tag_open                , std::bind(&token_iterator::on_tag_open_state                , this)},
     {tokenizer_state::end_tag_open            , std::bind(&token_iterator::on_end_tag_open_state            , this)},
     {tokenizer_state::tag_name                , std::bind(&token_iterator::on_tag_name_state                , this)},
     {tokenizer_state::before_attribute_name   , std::bind(&token_iterator::on_before_attribute_name_state   , this)},
     {tokenizer_state::attribute_name          , std::bind(&token_iterator::on_attribute_name_state          , this)},
     {tokenizer_state::before_attribute_value  , std::bind(&token_iterator::on_before_attribute_value_state  , this)},
     {tokenizer_state::unquoted_attribute_value, std::bind(&token_iterator::on_unquoted_attribute_value_state, this)},
     };

  bool token_emitted = false;
  while(!token_emitted) {
     if (state_handlers.find(this->state_) == state_handlers.end()) {
        throw std::runtime_error{"Unknown state."};
        }

     token_emitted = state_handlers[this->state_]();
     }

  return this->current_token_;
}

} // namespace HTML
