#include "cpputil.h"
#include "exceptions.h"
#include "valrep.h"
#include "function.h"
#include "array.h"
#include "string_literal.h"
#include "value.h"

Value::Value(int ival)
        : m_kind(VALUE_INT) {
    m_atomic.ival = ival;
}

Value::Value(Function *fn)
        : m_kind(VALUE_FUNCTION)
        , m_rep(fn) {
    m_rep = fn;
}

Value::Value(IntrinsicFn intrinsic_fn)
        : m_kind(VALUE_INTRINSIC_FN) {
    m_atomic.intrinsic_fn = intrinsic_fn;
}

Value::Value(Array *ar)
        : m_kind(VALUE_ARRAY)
        , m_rep(ar) {
    m_rep = ar;
}

Value::Value(String *st)
        : m_kind(VALUE_STRING)
        , m_rep(st) {
    m_rep = st;
}


Value::Value(const Value &other)
        : m_kind(VALUE_INT) {
    // Just use the assignment operator to copy the other Value's data
    *this = other;
}

Value::~Value() {
    // TODO: handle reference counting (detach from ValRep, if any)
}

Value &Value::operator=(const Value &rhs) {
    if (this != &rhs &&
        !(is_dynamic() && rhs.is_dynamic() && m_rep == rhs.m_rep)) {
        // TODO: handle reference counting (detach from previous ValRep, if any)
        m_kind = rhs.m_kind;
        if (is_dynamic()) {
            // attach to rhs's dynamic representation
            m_rep = rhs.m_rep;
            // TODO: handle reference counting (attach to the new ValRep)
        } else {
            // copy rhs's atomic representation
            m_atomic = rhs.m_atomic;
        }
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

std::string Value::as_str() const {
    switch (m_kind) {
        case VALUE_INT:
            return cpputil::format("%d", m_atomic.ival);
        case VALUE_FUNCTION:
            return cpputil::format("<function %s>", m_rep->as_function()->get_name().c_str());
        case VALUE_INTRINSIC_FN:
            return "<intrinsic function>";
        case VALUE_ARRAY: {
            std::string out;
            out.push_back('[');
            for (int i = 0; i < m_rep->as_array()->len().get_ival(); i++) {
                out.push_back(m_rep->as_array()->get(i).as_str().at(0));
                out.push_back(',');
                out.push_back(' ');
            }
            out.push_back(']');
            return out;
        }

        case VALUE_STRING:
            return cpputil::format("%s", m_rep->as_string()->get_str().c_str());
        default:
            // this should not happen
            RuntimeError::raise("Unknown value type %d", int(m_kind));
    }
}

String *Value::get_string() const {
    assert(m_kind == VALUE_STRING);
    return m_rep->as_string();
}

// TODO: implementations of additional member functions
