#include <cassert>
#include <algorithm>
#include <memory>
#include "ast.h"
#include "node.h"
#include "exceptions.h"
#include "function.h"
#include "interp.h"

Environment testing_env(nullptr);
Environment execution_env(nullptr);

Interpreter::Interpreter(Node *ast_to_adopt)
  : m_ast(ast_to_adopt) {
}

Interpreter::~Interpreter() {
  delete m_ast;
}

void Interpreter::analyze() {
    Environment semantic_env(&testing_env);
    search_for_semantic(m_ast, &semantic_env);

}

void Interpreter::search_for_semantic(Node *ast, Environment*test_env) {
    if (ast->get_tag() == AST_VARDEF) {
        test_env->new_variable(ast->get_last_kid()->get_str(), ast->get_last_kid()->get_loc());
    } else if (ast->get_tag() == AST_VARREF) {
        test_env->get_variable(ast->get_str(), ast->get_loc());
    }
    for (unsigned i = 0; i < ast->get_num_kids(); i++) {
        search_for_semantic(ast->get_kid(i), test_env);
    }
}

Value Interpreter::execute() {
  return execute_prime(m_ast);
}

Value Interpreter::execute_prime(Node * ast) {
    //TODO
    return {0};
}

Value Interpreter::define_variable(Node * ast){
    execution_env.new_variable(ast->get_last_kid()->get_str(), ast->get_last_kid()->get_loc());
    return {0};
}

Value Interpreter::get_variable(Node * ast){
    return {execution_env.get_variable(ast->get_str(), ast->get_loc())};
}

Value Interpreter::set_variable(Node * ast, int val) {
    execution_env.set_variable(ast->get_kid(0)->get_str(), val);
    return {val};
}

Value Interpreter::do_math(Node * ast) {
    int tag = ast->get_tag();

    int lhs = execute_prime(ast->get_kid(0)).get_ival();
    int rhs = execute_prime(ast->get_kid(1)).get_ival();

    switch (tag) {
        case AST_ADD:
            return {lhs + rhs};
        case AST_SUB:
            return {lhs - rhs};
        case AST_MULTIPLY:
            return {lhs * rhs};
        case AST_DIVIDE:
            if (rhs == 0) {
                EvaluationError::raise(ast->get_loc(), "Divide by 0");
            }
            return{lhs / rhs};
        default:
            EvaluationError::raise(ast->get_loc(), "Invalid math for operator %s", ast->get_str().c_str());
    }
}

Value Interpreter::binary_op(Node * ast) {

}

