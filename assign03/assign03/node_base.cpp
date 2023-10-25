// Copyright (c) 2021, David H. Hovemeyer <david.hovemeyer@gmail.com>
//
// Permission is hereby granted, free of charge, to any person obtaining a
// copy of this software and associated documentation files (the "Software"),
// to deal in the Software without restriction, including without limitation
// the rights to use, copy, modify, merge, publish, distribute, sublicense,
// and/or sell copies of the Software, and to permit persons to whom the
// Software is furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included
// in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
// THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
// OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
// ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
// OTHER DEALINGS IN THE SOFTWARE.

#include <cassert>
#include "node_base.h"

NodeBase::NodeBase() 
  : type(nullptr)
  , symb(nullptr) {
  
}

NodeBase::~NodeBase() {
}

// TODO: implement member functions

bool NodeBase::has_symbol() const {
  // if(symb){
  //     if(symb->get_parent()->has_symbol())
  // } else {
  //   return false;
  // }
 
  return symb != nullptr;
}

bool NodeBase::has_type() const {
  return type != nullptr;
}


void NodeBase::set_type(std::shared_ptr<Type> newType){
    // assert(!has_symbol());
    assert(!type);
    type = newType;
}

std::shared_ptr<Type> NodeBase::get_type() const {

 // if (has_symbol())
 //   return symb->get_type(); 
 // else {
    assert(type);
    return type;
 // }
}

void NodeBase::set_symbol(Symbol * variable_def){
  //assert(!has_symbol());
  assert(type == nullptr);
    symb = variable_def;
}

