//
// Created by root on 10/02/2022.
//

#ifndef COMPILERS_1_STRING_LITERAL_H
#define COMPILERS_1_STRING_LITERAL_H

#include "valrep.h"
#include "value.h"
#include <string>

class String : public ValRep {
private:
    explicit String();
    std::string m_string;
public:
    Value len() const;
    std::string get_str();

};


#endif //COMPILERS_1_STRING_LITERAL_H
