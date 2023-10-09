#include "function.h"
#include "array.h"
#include "string.h"
#include "valrep.h"

ValRep::ValRep(ValRepKind kind)
  : m_kind(kind)
  , m_refcount(0) {
}

ValRep::~ValRep() {
}

Function *ValRep::as_function() {
  assert(m_kind == VALREP_FUNCTION);
  return static_cast<Function *>(this);
}

Array *ValRep::as_array() {
  assert(m_kind == VALREP_ARRAY);
  return static_cast<Array *>(this);
}

String *ValRep::as_string(){
  assert(m_kind == VALREP_STRING);
  return static_cast<String *>(this);
}