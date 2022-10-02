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
    std::string m_string;
public:
    String();
    explicit String(std::string in);
    Value len() const;
    std::string get_str();
    void set_str(std::string in);
    void set_str(String in);
    //String strcat(String other);
    //String substr(int start, int end);

};


#endif //COMPILERS_1_STRING_LITERAL_H
