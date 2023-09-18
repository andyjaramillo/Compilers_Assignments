#include "environment.h"
#include "value.h"

Environment::Environment(Environment *parent)
  : m_parent(parent) {
  assert(m_parent != this);
}

Environment::~Environment() {
}

void Environment::assign_var_ref(std::string assign, int val){
    if(values.find(assign) != values.end()){
       values.at(assign) =  val;
    } 
}
void Environment::vardef_var_ref(std::string assign){
  Value varDef(0);
  values.insert({assign, varDef.get_kind()});
}
// TODO: implement member functions
