#ifndef ENVIRONMENT_H
#define ENVIRONMENT_H

#include <cassert>
#include <map>
#include <string>
#include "exceptions.h"
#include "value.h"
#include "node.h"

class Environment {
private:
  Environment *m_parent;
  // TODO: representation of environment (map of names to values)
  std::map<std::string, Value> values;
  // copy constructor and assignment operator prohibited
  Environment(const Environment &);
  Environment &operator=(const Environment &);

public:
  Environment(Environment *parent = nullptr);
  ~Environment();
  Value get_value_by_string(Node * lookUp);
  bool has_value_by_node(Node * lookUp);
  Environment * get_parent(){
    if(m_parent == nullptr){
      return this;
    } else {
      return m_parent;
    }
    };
  void assign_var_ref(std::string assign, Value  val);
  void bind(std::string assign, Value binded);
  void vardef_var_ref(std::string assign, Value k);
  void function_var_ref(std::string assign, Value * binded);
  void clear(){
    values.clear();
    };
  // TODO: add member functions allowing lookup, definition, and assignment
};

#endif // ENVIRONMENT_H
