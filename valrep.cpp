#include "function.h"
#include "valrep.h"
#include "Array.h"

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
