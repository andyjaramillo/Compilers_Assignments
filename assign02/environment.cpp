#include "environment.h"
#include "value.h"
#include <iostream>
Environment::Environment(Environment *parent)
  : m_parent(parent) {
  assert(m_parent != this);
}

Environment::~Environment() {
}

//get_value_by_string now recursively goes up and checks
//1. if it is in the current environment, return it
//2. if not, if this is not the parent, keep scaling up, otherwise stop since the parent becomes
//the global scope
Value Environment::get_value_by_string(Node * lookUp) {
    if(values.find(lookUp->get_str()) != values.end()){
      //we pass by reference so it can later be updated by assing_var_ref function
        return values.at(lookUp->get_str());
    } else {
      if(get_parent() == this) {
          SyntaxError::raise(lookUp->get_loc(), "Value not found");
      } else {
        return get_parent()->get_value_by_string(lookUp);
      }
    }

  }

bool Environment::has_value_by_node(Node * lookUp) {
    if(values.find(lookUp->get_str()) != values.end()){
      //we pass by reference so it can later be updated by assing_var_ref function
        return true;
    } else {
          return false;
    }

  }



void Environment::assign_var_ref(std::string assign, Value  val){
  if(values.find(assign) != values.end()){
      //we pass by reference so it can later be updated by assing_var_ref function
        this->values.at(assign) = val;
    } else {
      if(get_parent() == nullptr) {
            return;
      } else {
        get_parent()->assign_var_ref(assign, val);
      }
    }
}

void Environment::bind(std::string assign, Value binded){
   IntrinsicFn instrinsic = binded.get_intrinsic_fn();
    values.insert({assign, Value(instrinsic)});
}


void Environment::vardef_var_ref(std::string assign, Value k){
  values.insert({assign, k});
}

void Environment::function_var_ref(std::string assign , Value * binded){
  if(binded->get_kind() == VALUE_FUNCTION){
      values.insert({assign, Value(*binded)});
  }

}
// TODO: implement member functions
