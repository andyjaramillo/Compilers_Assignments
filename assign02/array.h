#ifndef ARRAY_H
#define ARRAY_H

#include <vector>
#include <string>
#include "valrep.h"
#include "value.h"
class Node;

class Array : public ValRep {
private:
  std::vector<Value> content;

  Array(const Array&);
  Array &operator=(const Array&);
  // value semantics prohibited
public:
  Array(const std::vector<Value> &content);
  virtual ~Array();
  std::vector<Value> &get_content();
  unsigned get_content_len();
  void set_content(Value element, unsigned index);
  void push_content(Value element);
  Value pop_content();
  Value get_content_element(unsigned index);
};

#endif // ARRAY_H