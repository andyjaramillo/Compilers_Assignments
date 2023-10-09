#include "cpputil.h"
#include "exceptions.h"
#include "valrep.h"
#include "function.h"
#include "array.h"
#include "string.h"
#include "value.h"
#include <iostream>

Value::Value(int ival)
  : m_kind(VALUE_INT) {
  m_atomic.ival = ival;
}

Value::Value(Function *fn)
  : m_kind(VALUE_FUNCTION)
  , m_rep(fn) {
  m_rep = fn;

}

Value::Value(Array *an)
  : m_kind(VALUE_ARRAY)
  , m_rep(an) {
  m_rep = an;
}

Value::Value(String *str)
  : m_kind(VALUE_STRING)
  , m_rep(str) {
  m_rep = str;
}

Value::Value(IntrinsicFn intrinsic_fn)
  : m_kind(VALUE_INTRINSIC_FN) {
  m_atomic.intrinsic_fn = intrinsic_fn;
}

Value::Value(const Value &other)
  : m_kind(VALUE_INT) {
  // Just use the assignment operator to copy the other Value's data
  *this = other;
}

Value::~Value() {
  // TODO: handle reference counting (detach from ValRep, if any)
    if(is_dynamic()) {
      if(m_rep->get_num_refs() == 0) {
          if(get_kind() == VALUE_FUNCTION) {
            Function * fn = get_function();
            delete fn;
          } else if(get_kind() == VALUE_ARRAY) {
            Array* arr = get_array();
            delete arr;
          } else if(get_kind() == VALUE_STRING){
            String* str = get_string();
            delete str;
          }
      } else {
        m_rep->remove_ref();
      }
       
          
    }
}

Value &Value::operator=(const Value &rhs) {
  if (this != &rhs &&
      !(is_dynamic() && rhs.is_dynamic() && m_rep == rhs.m_rep)) {
    // TODO: handle reference counting (detach from previous ValRep, if any)
    if(is_dynamic()){
      m_rep->remove_ref();
    }
    m_kind = rhs.m_kind;
    if (is_dynamic()) {
      // attach to rhs's dynamic representation
   //   std::cout << m_rep->get_num_refs() << std::endl;
  //    m_rep->remove_ref();
      m_rep = rhs.m_rep;
      // TODO: handle reference counting (attach to the new ValRep)
      rhs.m_rep->add_ref();
    } else {
      // copy rhs's atomic representation
      m_atomic = rhs.m_atomic;
    }
  } else if(is_dynamic() && rhs.is_dynamic() && this != &rhs){
    std::cout << "made it" << std::endl;
  }
  return *this;
}

Function *Value::get_function() const {
  assert(m_kind == VALUE_FUNCTION);
  return m_rep->as_function();
}

Array *Value::get_array() const {
  assert(m_kind == VALUE_ARRAY);
  return m_rep->as_array();
}

String *Value::get_string() const {
  assert(m_kind == VALUE_STRING);
  return m_rep->as_string();
}

std::string Value::as_str() const {
  /*
  the first character of the string is [
  
  the result of converting each element value to a string is appended, with each adjacent pair of elements being separated by a comma followed by a space
  
  the last character of the string is ]
  */
  switch (m_kind) {
  case VALUE_INT:
    return cpputil::format("%d", m_atomic.ival);
  case VALUE_FUNCTION:
    return cpputil::format("<function %s>", m_rep->as_function()->get_name().c_str());
  case VALUE_INTRINSIC_FN:
    return "<intrinsic function>";
  case VALUE_ARRAY:{
    std::vector<Value> arr = m_rep->as_array()->get_content();
    std::string str = "";
    for(unsigned i =0; i < arr.size(); i++){
      if(i == arr.size() -1){
         str = str + recursive_as_str(arr[i].get_kind(), arr[i]);
      } else{
          str = str + recursive_as_str(arr[i].get_kind(), arr[i]) + ", ";
      }
    }
    return cpputil::format("[%s]", str.c_str());
  }
  case VALUE_STRING:{
     std::vector<std::string> returned_string = m_rep->as_string()->get_sub_string();
     std::string str;
     for(unsigned i =0; i < returned_string.size(); i++){
      str = str + returned_string[i];
     }
     return cpputil::format("%s" , str.c_str());
  }
  default:
    // this should not happen
    RuntimeError::raise("Unknown value type %d", int(m_kind));
  }
}

std::string Value::recursive_as_str(ValueKind kind, Value element) const {
  switch (kind) {
    case VALUE_INT:
      return cpputil::format("%d", element.get_ival());
    case VALUE_FUNCTION:
      return cpputil::format("<function %s>", element.get_function()->get_name().c_str());
    case VALUE_INTRINSIC_FN:
      return "<intrinsic function>";
    case VALUE_ARRAY:{
      Array * array = element.get_array();
      std::vector<Value> arr = array->get_content();
      std::string str = "";
      for(unsigned i =0; i < arr.size(); i++){
        if(i == arr.size() -1){
            str = str + std::to_string(arr[i].get_ival());
        }
        str = str + std::to_string(arr[i].get_ival()) + ", ";
      }
      str.pop_back();
      return cpputil::format("[%s]", str.c_str());
    }
    case VALUE_STRING:{
      std::vector<std::string> returned_string = element.get_string()->get_sub_string();
     std::string str;
     for(unsigned i =0; i < returned_string.size(); i++){
      str = str + returned_string[i];
     }
        return cpputil::format("%s" , str.c_str());
    }
    default:
      // this should not happen
      RuntimeError::raise("Unknown value type %d", int(m_kind));
  }

}



// TODO: implementations of additional member functions
