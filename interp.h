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
  Interpreter(Node *ast_to_adopt);
  ~Interpreter();

  void analyze();
  Value execute();

private:
  // TODO: private member functions


    void search_for_semantic(Node *ast, Environment *test_env);

    Value do_math(Node * ast);

    Value binary_op(Node * ast);

    Value define_variable(Node *ast);

    Value get_variable(Node *ast);

    Value set_variable(Node *ast, int val);

    Value execute_prime(Node *ast);
};

#endif // INTERP_H
