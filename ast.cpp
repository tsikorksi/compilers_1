#include "exceptions.h"
#include "ast.h"

ASTTreePrint::ASTTreePrint() = default;

ASTTreePrint::~ASTTreePrint() = default;

std::string ASTTreePrint::node_tag_to_string(int tag) const {
    switch (tag) {
        case AST_UNIT:
            return "UNIT";
        case AST_STATEMENT:
            return "STATEMENT";
        case AST_ADD:
            return "ADD";
        case AST_SUB:
            return "SUB";
        case AST_MULTIPLY:
            return "MULTIPLY";
        case AST_DIVIDE:
            return "DIVIDE";
        case AST_VARREF:
            return "VARREF";
        case AST_VARDEF:
            return "VARDEF";
        case AST_INT_LITERAL:
            return "INT_LITERAL";
        case AST_ASSIGN:
            return "ASSIGN";
        case AST_AND:
            return "AND";
        case AST_OR:
            return "OR";
        case AST_LESS:
            return "LESS";
        case AST_LESSEQUAL:
            return "LESSEQUAL";
        case AST_GREATER:
            return "GREATER";
        case AST_GREATEREQUAL:
            return "GREATEREQUAL";
        case AST_EQUAL:
            return "EQUAL";
        case AST_NOTEQUAL:
            return "NOT";
        case AST_IF:
            return "IF";
        case AST_ELSE:
            return "ELSE";
        case AST_WHILE:
            return "WHILE";
        case AST_PARAMETER_LIST:
            return "PLIST";
        case AST_STATEMENT_LIST:
            return "SLIST";
        case AST_FUNCTION:
            return "FUNC";
        case AST_ARGLIST:
            return "ARGLIST";
        default:
            RuntimeError::raise("Unknown AST node type %d\n", tag);
    }
}
