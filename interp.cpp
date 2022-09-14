#include <cassert>
#include <algorithm>
#include <memory>
#include "ast.h"
#include "node.h"
#include "exceptions.h"
#include "function.h"
#include "interp.h"

Interpreter::Interpreter(Node *ast_to_adopt)
  : m_ast(ast_to_adopt) {
}

Interpreter::~Interpreter() {
  delete m_ast;
}

void Interpreter::analyze() {


}

void Interpreter::search_for_semantic(Node *ast, Environment test_env) {
    if (ast->get_tag() == AST_VARDEF) {
        test_env.new_variable(ast->get_str(), ast->get_loc());
    } else if (ast->get_tag() == AST_VARREF) {
        test_env.get_variable(ast->get_str(), ast->get_loc());
    }
    for (int i = 0; i < ast->get_num_kids(); i++) {
        search_for_semantic(ast->get_kid(i), test_env);
    }
}

Value Interpreter::execute() {
  // TODO: implement
  Value result;
  return result;
}

