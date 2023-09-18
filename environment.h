#ifndef ENVIRONMENT_H
#define ENVIRONMENT_H

#include <cassert>
#include <map>
#include <string>
#include "exceptions.h"
#include "value.h"

class Environment {
private:
  Environment *m_parent;
  // TODO: representation of environment (map of names to values)
  std::map<std::string, int> values;
  // copy constructor and assignment operator prohibited
  Environment(const Environment &);
  Environment &operator=(const Environment &);

public:
  Environment(Environment *parent = nullptr);
  ~Environment();
  int get_value_by_string(std::string lookUp) {
    if(values.find(lookUp) == values.end()){
       return -1;
    } else {
       return values.at(lookUp);
    }
  }
  void assign_var_ref(std::string assign, int val);
  void vardef_var_ref(std::string assign);
  void clear(){values.clear();};
  // TODO: add member functions allowing lookup, definition, and assignment
};

#endif // ENVIRONMENT_H
