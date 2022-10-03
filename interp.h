#ifndef INTERP_H
#define INTERP_H

#include "value.h"
#include "environment.h"
class Node;
class Location;

class Interpreter {
private:
    Node *m_ast;

public:
    explicit Interpreter(Node *ast_to_adopt);
    ~Interpreter();

    void analyze();
    Value execute();

private:

    void search_for_semantic(Node *ast, Environment *test_env);

    Value do_math(Node *ast, Environment *env);

    Value binary_op(Node *ast, Environment *env);

    static Value define_variable(Node *ast, Environment *env);

    static Value get_variable(Node *ast, Environment *env);

    Value execute_prime(Node *ast, Environment *env);

    static Value int_literal(Node *ast);

    void try_if(Node *ast, Environment *env);

    void try_while(Node *ast, Environment *env);

    static Value set_variable(Node *ast, const Value &val, Environment *env);


    Value call_intrinsic(Node *ast, Environment *env);

    void add_intrinsic(Environment * env);

    Value execute_statement_list(Node *ast, Environment *env);

    void check_condition(Node *ast, Environment *env);


    // I/O
    static Value intrinsic_println(Value *args, unsigned int num_args, const Location &loc, Interpreter *interp);
    static Value intrinsic_print(Value *args, unsigned int num_args, const Location &loc, Interpreter *interp);
    static Value intrinsic_readint(Value *args, unsigned int num_args, const Location &loc, Interpreter *interp);

    // Array funcs
    static Value intrinsic_mkarr(Value *args, unsigned int num_args, const Location &loc, Interpreter *interp);
    static Value intrinsic_len(Value *args, unsigned int num_args, const Location &loc, Interpreter *interp);
    static Value intrinsic_get(Value *args, unsigned int num_args, const Location &loc, Interpreter *interp);
    static Value intrinsic_set(Value *args, unsigned int num_args, const Location &loc, Interpreter *interp);
    static Value intrinsic_push(Value *args, unsigned int num_args, const Location &loc, Interpreter *interp);
    static Value intrinsic_pop(Value *args, unsigned int num_args, const Location &loc, Interpreter *interp);

    // String funcs
    static Value intrinsic_substr(Value *args, unsigned int num_args, const Location &loc, Interpreter *interp);
    static Value intrinsic_strcat(Value *args, unsigned int num_args, const Location &loc, Interpreter *interp);
    static Value intrinsic_strlen(Value *args, unsigned int num_args, const Location &loc, Interpreter *interp);

    void bind_params(Function * fn, Environment * env, Node * arg_list);


    static Value string_literal(Node *ast);
};

#endif // INTERP_H
