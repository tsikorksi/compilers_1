//
// Created by root on 10/02/2022.
//

#include "Array.h"


Array::Array() : ValRep(VALREP_ARRAY) {
}

void Array::push(const Value& val) {
    m_array.push_back(val);
}

Value Array::len() const {
    return {static_cast<int>(m_array.size())};
}

Value Array::set(int index, const Value& val) {
    m_array.at(index) = val;
    return {m_array.at(index)};
}

Value Array::get(int index) {
    return {m_array.at(index)};
}

Value Array::pop() {
    Value back = m_array.back();
    m_array.pop_back();
    return {back};
}


