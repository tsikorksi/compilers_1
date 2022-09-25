#include <algorithm>
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

void Interpreter::add_intrinsic() {
    global_env.bind("print",m_ast->get_loc() , Value(&intrinsic_print));
    global_env.bind("println",m_ast->get_loc() , Value(&intrinsic_println));
}

Value Interpreter::execute() {
    add_intrinsic();
    return execute_prime(m_ast);
}

Value Interpreter::execute_prime(Node * ast) {
    int tag = ast->get_tag();
    Value final;
    switch (ast->get_tag()) {
        case AST_STATEMENT_LIST:
        case AST_UNIT:
            for (unsigned i = 0; i < ast->get_num_kids() ;i++) {
                final = execute_prime(ast->get_kid(i));
            }
            return final;
        case AST_STATEMENT:
            return execute_prime(ast->get_kid(0));
        case AST_IF:
            try_if(ast);
            // control flow evaluates to 0
            return {0};
        case AST_WHILE:
            try_while(ast);
            // control flow evaluates to 0
            return {0};
        case AST_VARREF:
            return get_variable(ast);
        case AST_VARDEF:
            return define_variable(ast);
        case AST_INT_LITERAL:
            return int_literal(ast);
        case AST_ASSIGN:
            return set_variable(ast->get_kid(0), execute_prime(ast->get_kid(1)));
        case AST_AND:
        case AST_OR:
        case AST_LESS:
        case AST_LESSEQUAL:
        case AST_GREATER:
        case AST_GREATEREQUAL:
        case AST_EQUAL:
        case AST_NOTEQUAL:
            return binary_op(ast);
        case AST_ADD:
        case AST_SUB:
        case AST_MULTIPLY:
        case AST_DIVIDE:
            return do_math(ast);
        default:
            RuntimeError::raise("Unknown AST node type %d\n", tag);
    }
}

void Interpreter::try_if(Node *ast) {
    Value condition = execute_prime(ast->get_kid(0));

    if (condition.is_numeric()){
        if (condition.get_ival()){
            execute_prime(ast->get_kid(1));
        } else {
            if (ast->get_num_kids() == 3) {
                execute_prime(ast->get_kid(2));
            }
        }
    } else {
        EvaluationError::raise(ast->get_loc(), "If Statement condition is not numeric");

    }
}

void Interpreter::try_while(Node *ast) {
    Value condition = execute_prime(ast->get_kid(0));
    if (condition.is_numeric()) {
        while (condition.get_ival()) {
            execute_prime(ast->get_kid(1));
            condition = execute_prime(ast->get_kid(0));
        }
    } else {
        EvaluationError::raise(ast->get_loc(), "While Statement condition is not numeric");
    }
}


Value Interpreter::define_variable(Node * ast){
    global_env.new_variable(ast->get_last_kid()->get_str(), ast->get_last_kid()->get_loc());
    return {0};
}

Value Interpreter::get_variable(Node * ast){
    return {global_env.get_variable(ast->get_str(), ast->get_loc())};
}

Value Interpreter::set_variable(Node * ast, const Value& val) {
    global_env.set_variable(ast->get_str(), val);
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

    int tag = ast->get_tag();
    int lhs = execute_prime(ast->get_kid(0)).get_ival();

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

    int rhs = execute_prime(ast->get_kid(1)).get_ival();

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
    printf("%s", args[0].as_str().c_str());
    return {};
}

Value Interpreter::intrinsic_println(Value *args, unsigned int num_args, const Location &loc, Interpreter *interp) {
    intrinsic_print(args, num_args, loc, interp);
    printf("\n");
    return {};
}




