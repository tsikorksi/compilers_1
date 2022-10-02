//
// Created by root on 10/02/2022.
//

#include "string_literal.h"

String::String() : ValRep(VALREP_STRING){

}

//String::String(std::string input) {
//    m_string = input;
//}

Value String::len() const {
    return {static_cast<int>(m_string.length())};
}

std::string String::get_str() {
    return m_string;
}





//String String::strcat(String other) {
//    std::string new_string = m_string + other.m_string;
//    return String(new_string);
//}

//String String::substr(int start, int end) {
//   return String();
//}
