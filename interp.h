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

    Value do_math(Node * ast);

    Value binary_op(Node * ast);

    static Value define_variable(Node *ast);

    static Value get_variable(Node *ast);

    Value execute_prime(Node *ast);

    static Value int_literal(Node *ast);

    void try_if(Node *pNode);

    void try_while(Node *pNode);

    static Value set_variable(Node *ast, const Value& val);

    void add_intrinsic();

    static Value intrinsic_print(Value *args, unsigned int num_args, const Location &loc, Interpreter *interp);

    static Value intrinsic_println(Value *args, unsigned int num_args, const Location &loc, Interpreter *interp);
};

#endif // INTERP_H
