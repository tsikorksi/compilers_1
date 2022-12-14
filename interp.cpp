#include <functional>
#include <memory>
#include "ast.h"
#include "node.h"
#include "exceptions.h"
#include "function.h"
#include "interp.h"
#include "array.h"
#include "string_literal.h"
#include "intrinsic.h"


std::unique_ptr<Environment> testing_env(new Environment(nullptr));
std::unique_ptr<Environment> global_env(new Environment(nullptr));

Interpreter::Interpreter(Node *ast_to_adopt)
        : m_ast(ast_to_adopt) {
}

Interpreter::~Interpreter() {
    delete m_ast;
}

void Interpreter::analyze() {
    add_intrinsic(testing_env.get());
    search_for_semantic(m_ast, testing_env.get());
}

void Interpreter::search_for_semantic(Node *ast, Environment *test_env) {
    int tag = ast->get_tag();

    switch (tag) {
        case AST_VARDEF:
            test_env->new_variable(ast->get_last_kid()->get_str(), ast->get_last_kid()->get_loc(), VALUE_INT);
        case AST_VARREF:
            test_env->get_variable(ast->get_str(), ast->get_loc());
        case AST_UNIT:
        case AST_STATEMENT_LIST: {
            std::unique_ptr<Environment> new_env(new Environment(test_env));
            for (unsigned i = 0; i < ast->get_num_kids(); i++) {
                search_for_semantic(ast->get_kid(i), new_env.get());
            }
        }
        default:
            break;
    }

}

void Interpreter::add_intrinsic(Environment *env) {
    // From intrinsic.cpp
    // Bind all intrinsic functions
    // I/O
    env->bind("print", m_ast->get_loc(), IntrinsicFn(intrinsic_print));
    env->bind("println", m_ast->get_loc(), IntrinsicFn(intrinsic_println));
    env->bind("readint", m_ast->get_loc(), IntrinsicFn(intrinsic_readint));
    // Arrays
    env->bind("mkarr", m_ast->get_loc(), IntrinsicFn(intrinsic_mkarr));
    env->bind("len", m_ast->get_loc(), IntrinsicFn(intrinsic_len));
    env->bind("get", m_ast->get_loc(), IntrinsicFn(intrinsic_get));
    env->bind("set", m_ast->get_loc(), IntrinsicFn(intrinsic_set));
    env->bind("pop", m_ast->get_loc(), IntrinsicFn(intrinsic_pop));
    env->bind("push", m_ast->get_loc(), IntrinsicFn(intrinsic_push));
    // Strings
    env->bind("strlen", m_ast->get_loc(), IntrinsicFn(intrinsic_strlen));
    env->bind("strcat", m_ast->get_loc(), IntrinsicFn(intrinsic_strcat));
    env->bind("substr", m_ast->get_loc(), IntrinsicFn(intrinsic_substr));
}

Value Interpreter::execute() {
    add_intrinsic(global_env.get());
    return execute_prime(m_ast, global_env.get());
}

Value Interpreter::execute_prime(Node *ast, Environment *env) {
    int tag = ast->get_tag();

    switch (ast->get_tag()) {
        case AST_STATEMENT_LIST: {
            // enter new scope
            Environment new_env(env);

            Value final = execute_statement_list(ast, &new_env);
            return final;
        }
        case AST_UNIT:
            return execute_statement_list(ast, env);
        case AST_STATEMENT:
            return execute_prime(ast->get_kid(0), env);
        case AST_IF:
            try_if(ast, env);
            // control flow evaluates to 0
            return {0};
        case AST_WHILE:
            try_while(ast, env);
            // control flow evaluates to 0
            return {0};
        case AST_VARREF:
            if (ast->get_num_kids() != 0 && ast->get_kid(0)->get_tag() == AST_ARGLIST) {
                switch (get_variable(ast, env).get_kind()) {

                    case VALUE_INTRINSIC_FN:
                        return call_intrinsic(ast, env);
                    case VALUE_FUNCTION: {
                        // extract function from environment
                        Function *fn = get_variable(ast, env).get_function();
                        Environment func_env(fn->get_parent_env());
                        // bind the parameters to the arguments within the function scope
                        // add local env as the execution env of the arguments
                        bind_params(fn, &func_env, env, ast->get_kid(0));
                        // execute the ast tree shard
                        return execute_prime(fn->get_body(), &func_env);
                    }
                    case VALUE_ARRAY:
                    case VALUE_STRING:
                    case VALUE_INT:
                        EvaluationError::raise(ast->get_loc(), "Non-function variable given arguments");
                }
            }
            return get_variable(ast, env);
        case AST_VARDEF:
            return define_variable(ast, env);
        case AST_FUNCTION: {
            std::vector<std::string> params;
            // add list of param strings
            for (unsigned i = 0; i < ast->get_kid(1)->get_num_kids(); i++) {
                params.push_back(ast->get_kid(1)->get_kid(i)->get_str());
            }
            Value fn_val(new Function(ast->get_kid(0)->get_str(), params, global_env.get(), ast->get_kid(2)));
            env->bind(fn_val.get_function()->get_name(), ast->get_loc(), fn_val);
            return {};
        }
        case AST_INT_LITERAL:
            return int_literal(ast);
        case AST_STRING:
            return string_literal(ast);
        case AST_ASSIGN:
            return set_variable(ast->get_kid(0), execute_prime(ast->get_kid(1), env), env);
        case AST_ARGLIST: {
            EvaluationError::raise(ast->get_loc(), "Argument list made child of non function call");
        }
        case AST_PARAMETER_LIST: {
            EvaluationError::raise(ast->get_loc(), "Parameter list made child of non function call");
        }
        case AST_AND:
        case AST_OR:
        case AST_LESS:
        case AST_LESSEQUAL:
        case AST_GREATER:
        case AST_GREATEREQUAL:
        case AST_EQUAL:
        case AST_NOTEQUAL:
            return binary_op(ast, env);
        case AST_ADD:
        case AST_SUB:
        case AST_MULTIPLY:
        case AST_DIVIDE:
            return do_math(ast, env);
        default:
            RuntimeError::raise("Unknown AST node type %d\n", tag);
    }
}

Value Interpreter::execute_statement_list(Node *ast, Environment *env) {
    Value final;

    for (unsigned i = 0; i < ast->get_num_kids(); i++) {
        final = execute_prime(ast->get_kid(i), env);
    }
    return final;
}

void Interpreter::bind_params(Function *fn, Environment *env, Environment *local_env, Node *arg_list) {
    std::vector<std::string> params = fn->get_params();

    if (fn->get_params().size() != arg_list->get_num_kids()) {
        EvaluationError::raise(arg_list->get_loc(), "Wrong number of arguments to function %s", fn->get_name().c_str());
    }

    // bind all parameters to passed in values
    for (long unsigned int i = 0; i < params.size(); i++) {
        // local env is where each argument is evaluated, env is where the params will be bound
        env->bind(params.at(i), arg_list->get_loc(), execute_prime(arg_list->get_kid(i), local_env));
    }
}

void Interpreter::try_if(Node *ast, Environment *env) {
    check_condition(ast, env);
    if (execute_prime(ast->get_kid(0), env).get_ival()) {
        execute_prime(ast->get_kid(1), env);

    } else {
        // see if there
        if (ast->get_num_kids() == 3) {
            execute_prime(ast->get_kid(2), env);
        }
    }

}

void Interpreter::try_while(Node *ast, Environment *env) {
    check_condition(ast, env);

    Value condition = execute_prime(ast->get_kid(0), env);

    while (condition.get_ival()) {
        execute_prime(ast->get_kid(1), env);
        condition = execute_prime(ast->get_kid(0), env);
    }
}

void Interpreter::check_condition(Node *ast, Environment *env) {

    Value kind = execute_prime(ast->get_kid(0), env);
    // check we are using an int as a condition
    if (kind.is_numeric()) {
        return;
    }
    EvaluationError::raise(ast->get_loc(), "Statement condition is not numeric");
}


Value Interpreter::define_variable(Node *ast, Environment *env) {
    env->new_variable(ast->get_last_kid()->get_str(), ast->get_last_kid()->get_loc(), VALUE_INT);
    return {0};
}

Value Interpreter::get_variable(Node *ast, Environment *env) {
    return env->get_variable(ast->get_str(), ast->get_loc());
}

Value Interpreter::set_variable(Node *ast, const Value &val, Environment *env) {
    env->set_variable(ast->get_str(), val, ast->get_loc());
    return {val};
}

Value Interpreter::do_math(Node *ast, Environment *env) {
    int tag = ast->get_tag();

    Value lhs_val = execute_prime(ast->get_kid(0), env);
    Value rhs_val = execute_prime(ast->get_kid(1), env);

    // verify we are doing math on ints
    if (!lhs_val.is_numeric() || !rhs_val.is_numeric()) {
        EvaluationError::raise(ast->get_loc(), "Operand is not numeric");
    }

    int lhs = lhs_val.get_ival();
    int rhs = rhs_val.get_ival();

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
            return {lhs / rhs};
        default:
            EvaluationError::raise(ast->get_loc(), "Invalid math for operator %s", ast->get_str().c_str());
    }
}

Value Interpreter::binary_op(Node *ast, Environment *env) {

    int tag = ast->get_tag();
    Value lhs_val = execute_prime(ast->get_kid(0), env);

    // check something weird isn't being passed in
    if (!lhs_val.is_numeric()) {
        EvaluationError::raise(ast->get_loc(), "%s passed into binary operation", lhs_val.as_str().c_str());
    }
    int lhs = lhs_val.get_ival();

    // Short circuit the OR and AND binary operations
    switch (tag) {
        case AST_AND:
            if (lhs == 0) {
                return {0};
            }
            break;
        case AST_OR:
            if (lhs == 1) {
                return {1};
            }
            break;
        default:
            break;
    }

    Value rhs_val = execute_prime(ast->get_kid(1), env);

    // check something weird isn't being passed in
    if (!rhs_val.is_numeric()) {
        EvaluationError::raise(ast->get_loc(), "%s passed into binary operation", rhs_val.as_str().c_str());
    }
    int rhs = rhs_val.get_ival();

    switch (tag) {
        case AST_AND:
            if (lhs == 1 && rhs == 1) {
                return {1};
            }
            break;
        case AST_OR:
            if (lhs != 0 || rhs != 0) {
                return {1};
            }
            break;
        case AST_LESS:
            if (lhs < rhs) {
                return {1};
            }
            break;
        case AST_LESSEQUAL:
            if (lhs <= rhs) {
                return {1};
            }
            break;
        case AST_GREATER:
            if (lhs > rhs) {
                return {1};
            }
            break;
        case AST_GREATEREQUAL:
            if (lhs >= rhs) {
                return {1};
            }
            break;
        case AST_EQUAL:
            if (lhs == rhs) {
                return {1};
            }
            break;
        case AST_NOTEQUAL:
            if (lhs != rhs) {
                return {1};
            }
            break;
        default:
            EvaluationError::raise(ast->get_loc(), "Invalid binary math for operator %s", ast->get_str().c_str());
    }
    return {0};
}

Value Interpreter::int_literal(Node *ast) {
    return {std::stoi(ast->get_str())};
}

Value Interpreter::string_literal(Node *ast) {
    return new String(ast->get_str());
}


Value Interpreter::call_intrinsic(Node *ast, Environment *env) {

    std::string name = ast->get_str();
    Node *arg_list = ast->get_kid(0);
    Value args[arg_list->get_num_kids()];

    for (unsigned i = 0; i < arg_list->get_num_kids(); i++) {
        args[i] = execute_prime(arg_list->get_kid(i), env);
    }

    Value fn = get_variable(ast, env);
    IntrinsicFn fp = fn.get_intrinsic_fn();
    return fp(args, arg_list->get_num_kids(), ast->get_loc());
}




