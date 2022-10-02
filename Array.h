//
// Created by root on 10/02/2022.
//

#ifndef COMPILERS_1_ARRAY_H
#define COMPILERS_1_ARRAY_H

#include "valrep.h"
#include "value.h"
#include <vector>


class Array : public  ValRep{
private:
    explicit Array();

    std::vector<Value> m_array;
public:

    Value len() const;
    void push(const Value &val);
    Value set(int index, const Value& val);
    Value get(int index);
    Value pop();
};


#endif //COMPILERS_1_ARRAY_H
