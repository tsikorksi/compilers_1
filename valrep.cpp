#include "function.h"
#include "valrep.h"
#include "string_literal.h"
#include "array.h"

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

String *ValRep::as_string() {
    assert(m_kind == VALREP_STRING);
    return static_cast<String *>(this);
}

