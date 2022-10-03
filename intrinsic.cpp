//
// Created by root on 10/03/2022.
//

#include <iostream>
#include "intrinsic.h"
#include "exceptions.h"
#include "value.h"
#include "string_literal.h"
#include "array.h"

Value intrinsic_print(Value args[], unsigned num_args, const Location &loc) {
    if (num_args != 1)
        EvaluationError::raise(loc, "Wrong number of arguments passed to print function");
    std::cout << args[0].as_str();
    return {};
}

Value intrinsic_println(Value args[], unsigned int num_args, const Location &loc) {
    intrinsic_print(args, num_args, loc);
    std::cout << "\n";
    return {};
}

Value intrinsic_readint(Value args[], unsigned int num_args, const Location &loc) {
    if (num_args != 0)
        EvaluationError::raise(loc, "Wrong number of arguments passed to readint function");
    int input = 0;
    int result = scanf("%i", &input);
    if (result == 0) {
        EvaluationError::raise(loc, "No input detected");
    } else if (result == EOF) {
        EvaluationError::raise(loc, "Unexpected End Of File");
    }
    return {input};
}


Value intrinsic_strlen(Value *args, unsigned int num_args, const Location &loc) {
    if (num_args != 1)
        EvaluationError::raise(loc, "Wrong number of arguments passed to strlen function");
    if (args[0].get_kind() != VALUE_STRING)
        EvaluationError::raise(loc, "strlen function not passed String");
    return args[0].get_string()->len();
}

Value intrinsic_strcat(Value *args, unsigned int num_args, const Location &loc) {
    if (num_args != 2)
        EvaluationError::raise(loc, "Wrong number of arguments passed to strcat function");
    if (args[0].get_kind() != VALUE_STRING)
        EvaluationError::raise(loc, "strcat function not passed String in first argument");
    if (args[1].get_kind() != VALUE_STRING)
        EvaluationError::raise(loc, "strcat function not passed String in second argument");
    return new String(args[0].get_string()->get_str() + args[1].get_string()->get_str());
}

Value intrinsic_substr(Value *args, unsigned int num_args, const Location &loc) {
    if (num_args != 3)
        EvaluationError::raise(loc, "Wrong number of arguments passed to substr function");
    if (args[0].get_kind() != VALUE_STRING)
        EvaluationError::raise(loc, "substr function not passed String in first argument");
    if (!args[1].is_numeric())
        EvaluationError::raise(loc, "substr function not passed int in second argument");
    if (!args[2].is_numeric())
        EvaluationError::raise(loc, "substr function not passed int in third argument");
    return new String(args[0].get_string()->get_str().substr(args[1].get_ival(), args[2].get_ival()));
}


Value intrinsic_mkarr(Value args[], unsigned int num_args, const Location &loc) {
    Value arr = new Array();
    for (unsigned int i = 0; i < num_args; i++) {
        arr.get_array()->push(args[i]);
    }
    return arr;
}

Value intrinsic_len(Value *args, unsigned int num_args, const Location &loc) {
    if (num_args != 1)
        EvaluationError::raise(loc, "Wrong number of arguments to length check call");
    if (args[0].get_kind() != VALUE_ARRAY)
        EvaluationError::raise(loc, "Length call not passed array type");

    return args[0].get_array()->len();
}

Value intrinsic_get(Value *args, unsigned int num_args, const Location &loc) {
    if (num_args != 2)
        EvaluationError::raise(loc, "Wrong number of arguments to array get call");
    if (args[0].get_kind() != VALUE_ARRAY)
        EvaluationError::raise(loc, "Get call not passed array type");
    if (args[1].get_kind() != VALUE_INT)
        EvaluationError::raise(loc, "Get call not passed int for index");
    return args[0].get_array()->get(args[1].get_ival());

}

Value intrinsic_set(Value *args, unsigned int num_args, const Location &loc) {
    if (num_args != 3)
        EvaluationError::raise(loc, "Wrong number of arguments to array set call");
    if (args[0].get_kind() != VALUE_ARRAY)
        EvaluationError::raise(loc, "Set call not passed array type");
    if (args[1].get_kind() != VALUE_INT)
        EvaluationError::raise(loc, "Set call not passed int for index");
    args[0].get_array()->set(args[1].get_ival(), args[2]);
    return args[2];
}

Value intrinsic_push(Value *args, unsigned int num_args, const Location &loc) {
    if (num_args != 2)
        EvaluationError::raise(loc, "Wrong number of arguments to array push call");
    if (args[0].get_kind() != VALUE_ARRAY)
        EvaluationError::raise(loc, "Push call not passed array type");
    args[0].get_array()->push(args[1]);
    return args[1];
}

Value intrinsic_pop(Value *args, unsigned int num_args, const Location &loc) {
    if (num_args != 1)
        EvaluationError::raise(loc, "Wrong number of arguments to array pop call");
    if (args[0].get_kind() != VALUE_ARRAY)
        EvaluationError::raise(loc, "Pop call not passed array type");
    return args[0].get_array()->pop();
}
