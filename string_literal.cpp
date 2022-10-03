//
// Created by root on 10/02/2022.
//

#include "string_literal.h"
#include <string>
#include <utility>


String::String() : ValRep(VALREP_STRING) {

}

String::String(std::string in) : ValRep(VALREP_STRING) {
    m_string = std::move(in);
}

Value String::len() const {
    return {static_cast<int>(m_string.length())};
}

std::string String::get_str() {
    return m_string;
}

void String::set_str(std::string in) {
    m_string = std::move(in);
}

void String::set_str(String in) {
    this->m_string = in.m_string;
}

