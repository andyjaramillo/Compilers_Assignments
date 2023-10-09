#ifndef STRING_H
#define STRING_H

#include <vector>
#include <string>
#include "valrep.h"
#include "value.h"
class Node;

class String : public ValRep {
private:
  std::vector<std::string> sub_string;

  String(const String&);
  String &operator=(const String&);
  std::vector<std::string> format_string(std::string non_format_string);
  // value semantics prohibited
public:
  String(const std::vector<std::string> &sub_string);
  String(std::string &formatted_string);
  virtual ~String();
  std::vector<std::string> &get_sub_string();
  unsigned get_sub_string_len();
};

#endif // ARRAY_H