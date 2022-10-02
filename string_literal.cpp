//
// Created by root on 10/02/2022.
//

#include "string_literal.h"

String::String() : ValRep(VALREP_STRING){

}

Value String::len() const {
    return {static_cast<int>(m_string.length())};
}

std::string String::get_str() {
    return m_string;
}
