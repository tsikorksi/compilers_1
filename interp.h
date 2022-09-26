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
    // TODO: private member functions


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

    static IntrinsicFn intrinsic_print(Value *args, unsigned int num_args, const Location &loc);

    static IntrinsicFn intrinsic_println(Value *args, unsigned int num_args, const Location &loc);

    Value call_intrinsic(Node *ast, Environment *env);

    void add_intrinsic(Environment * env);

    Value execute_statement_list(Node *ast, Environment *env);
};

#endif // INTERP_H
