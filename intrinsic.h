//
// Created by root on 10/03/2022.
//

#ifndef COMPILERS_1_INTRINSIC_H
#define COMPILERS_1_INTRINSIC_H

#include "value.h"

// I/O
extern Value intrinsic_println(Value *args, unsigned int num_args, const Location &loc);
extern Value intrinsic_print(Value *args, unsigned int num_args, const Location &loc);
extern Value intrinsic_readint(Value *args, unsigned int num_args, const Location &loc);

// Array funcs
extern Value intrinsic_mkarr(Value *args, unsigned int num_args, const Location &loc);
extern Value intrinsic_len(Value *args, unsigned int num_args, const Location &loc);
extern Value intrinsic_get(Value *args, unsigned int num_args, const Location &loc);
extern Value intrinsic_set(Value *args, unsigned int num_args, const Location &loc);
extern Value intrinsic_push(Value *args, unsigned int num_args, const Location &loc);
extern Value intrinsic_pop(Value *args, unsigned int num_args, const Location &loc);

// String funcs
extern Value intrinsic_substr(Value *args, unsigned int num_args, const Location &loc);
extern Value intrinsic_strcat(Value *args, unsigned int num_args, const Location &loc);
extern Value intrinsic_strlen(Value *args, unsigned int num_args, const Location &loc);

#endif //COMPILERS_1_INTRINSIC_H
