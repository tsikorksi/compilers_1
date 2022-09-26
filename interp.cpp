#include <functional>
#include <iostream>
#include "ast.h"
#include "node.h"
#include "exceptions.h"
#include "function.h"
#include "interp.h"

Environment testing_env(nullptr);
Environment global_env(nullptr);

Interpreter::Interpreter(Node *ast_to_adopt)
        : m_ast(ast_to_adopt) {
}

Interpreter::~Interpreter() {
    delete m_ast;
}

void Interpreter::analyze() {
    add_intrinsic(&testing_env);
    search_for_semantic(m_ast, &testing_env);
}

void Interpreter::search_for_semantic(Node *ast, Environment*test_env) {
    int tag = ast->get_tag();

    switch (tag) {
        case AST_VARDEF:
            test_env->new_variable(ast->get_last_kid()->get_str(), ast->get_last_kid()->get_loc(), VALUE_INT);
        case AST_VARREF:
            test_env->get_variable(ast->get_str(), ast->get_loc());
        case AST_UNIT:
        case AST_STATEMENT_LIST: {
            auto *new_env = new Environment(test_env);
            for (unsigned i = 0; i < ast->get_num_kids(); i++) {
                search_for_semantic(ast->get_kid(i), new_env);
            }
        }
        default:
            break;
    }

}

void Interpreter::add_intrinsic(Environment * env) {
    env->bind("print",m_ast->get_loc() , Value(&intrinsic_print));
    env->bind("println",m_ast->get_loc() , Value(&intrinsic_println));
}

Value Interpreter::execute() {
    add_intrinsic(&global_env);
    return execute_prime(m_ast, &global_env);
}

Value Interpreter::execute_prime(Node *ast, Environment *env) {
    int tag = ast->get_tag();

    switch (ast->get_tag()) {
        case AST_STATEMENT_LIST:
        {
            auto *new_env = new Environment(env);

            Value final = execute_statement_list(ast, new_env);
            //delete new_env;
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
                    case VALUE_INT:
                        EvaluationError::raise(ast->get_loc(), "Non-function variable given arguments");
                    case VALUE_INTRINSIC_FN:
                        call_intrinsic(ast, env);
                        return {0};
                    case VALUE_FUNCTION:
                        //TODO: user generated functions
                        return {0};
                }
            }
            return get_variable(ast, env);
        case AST_VARDEF:
            return define_variable(ast, env);
        case AST_INT_LITERAL:
            return int_literal(ast);
        case AST_ASSIGN:
            return set_variable(ast->get_kid(0), execute_prime(ast->get_kid(1), env), env);
        case AST_ARGLIST:
            //TODO: convert to generating list
            return execute_prime(ast->get_kid(0), env);
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

Value Interpreter::execute_statement_list(Node* ast, Environment* env) {
    Value final;

    for (unsigned i = 0; i < ast->get_num_kids(); i++) {
        final = execute_prime(ast->get_kid(i), env);
    }
    return final;
}

void Interpreter::try_if(Node *ast, Environment *env) {
    Value condition = execute_prime(ast->get_kid(0), env);

    if (condition.is_numeric()){
        if (condition.get_ival()){
            execute_prime(ast->get_kid(1), env);

        } else {
            if (ast->get_num_kids() == 3) {
                execute_prime(ast->get_kid(2), env);
            }
        }
    } else {
        EvaluationError::raise(ast->get_loc(), "If Statement condition is not numeric");

    }
}

void Interpreter::try_while(Node *ast, Environment *env) {
    Value condition = execute_prime(ast->get_kid(0), env);
    if (condition.is_numeric()) {
        while (condition.get_ival()) {
            execute_prime(ast->get_kid(1), env);
            condition = execute_prime(ast->get_kid(0), env);
        }
    } else {
        EvaluationError::raise(ast->get_loc(), "While Statement condition is not numeric");
    }
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
            return{lhs / rhs};
        default:
            EvaluationError::raise(ast->get_loc(), "Invalid math for operator %s", ast->get_str().c_str());
    }
}

Value Interpreter::binary_op(Node *ast, Environment *env) {

    int tag = ast->get_tag();
    Value lhs_val = execute_prime(ast->get_kid(0), env);

    // check something weird isn't being passed in
    if (!lhs_val.is_numeric()){
        EvaluationError::raise(ast->get_loc(),"%s passed into binary operation", lhs_val.as_str().c_str());
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
    if (!rhs_val.is_numeric()){
        EvaluationError::raise(ast->get_loc(),"%s passed into binary operation", rhs_val.as_str().c_str());
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

Value Interpreter::int_literal(Node * ast) {
    return {std::stoi(ast->get_str())};
}


Value Interpreter::intrinsic_print(Value args[], unsigned num_args,
                                   const Location &loc, Interpreter *interp) {
    if (num_args != 1)
        EvaluationError::raise(loc, "Wrong number of arguments passed to print function");
    std::cout << args[0].as_str().c_str();
    return {};
}

Value Interpreter::intrinsic_println(Value *args, unsigned int num_args, const Location &loc, Interpreter *interp) {
    intrinsic_print(args, num_args, loc, interp);
    std::cout << "\n";
    return {};
}

Value Interpreter::call_intrinsic(Node *ast, Environment *env) {

    //TODO: have it actually pull from global_env
    std::string name = ast->get_str();
    Value arg = execute_prime(ast->get_kid(0), env);

    Value  args[1] = {arg};

    if (name == "print") {
        return intrinsic_print(args, 1, ast->get_loc(), this);
    } else if (name == "println") {
        return intrinsic_println(args, 1, ast->get_loc(), this);
    }
    return {0};
}




