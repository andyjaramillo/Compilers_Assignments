#include "array.h"
#include "value.h"
Array::Array(const std::vector<Value> &content)
  : ValRep(VALREP_ARRAY)
  , content(content) {
}


Array::~Array() {
}

std::vector<Value> &Array::get_content(){
  return content;
}

unsigned Array::get_content_len(){
  return content.size();
}

Value Array::get_content_element(unsigned index){
  return content.at(index);
}

void Array::set_content(Value element, unsigned index){
  content.at(index) = element;
}

void Array::push_content(Value element){
  content.push_back(element);
}

Value Array::pop_content(){
  Value popped = content.back();
  content.pop_back();
  return popped;
}


// TODO: implement member functions