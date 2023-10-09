#ifndef LITERAL_VALUE_H
#define LITERAL_VALUE_H

#include <cstdint>
#include "location.h"

enum class LiteralValueKind {
  NONE,
  INTEGER,
  CHARACTER,
  STRING,
};

// Helper class for representing literal values (integer, character,
// and string)
class LiteralValue {
private:
  LiteralValueKind m_kind;
  int64_t m_intval;
  std::string m_strval; // this is also used for character literals

  // integer literals can be specified as being unsigned and/or long
  bool m_is_unsigned;
  bool m_is_long;

public:
  LiteralValue();
  LiteralValue(int64_t val, bool is_unsigned, bool is_long);
  LiteralValue(char c);
  LiteralValue(const std::string &s);
  LiteralValue(const LiteralValue &other);
  ~LiteralValue();

  LiteralValue &operator=(const LiteralValue &rhs);

  LiteralValueKind get_kind() const;

  int64_t get_int_value() const;
  char get_char_value() const;
  std::string get_str_value() const;

  bool is_unsigned() const;
  bool is_long() const;

  // helper functions to create a LiteralValue from a token lexeme
  static LiteralValue from_char_literal(const std::string &lexeme, const Location &loc);
  static LiteralValue from_int_literal(const std::string &lexeme, const Location &loc);
  static LiteralValue from_str_literal(const std::string &lexeme, const Location &loc);

private:
  static std::string strip_quotes(const std::string &lexeme, char quote);
};

#endif // LITERAL_VALUE_H
