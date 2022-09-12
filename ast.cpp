#include "exceptions.h"
#include "ast.h"

ASTTreePrint::ASTTreePrint() {
}

ASTTreePrint::~ASTTreePrint() {
}

std::string ASTTreePrint::node_tag_to_string(int tag) const {
    switch (tag) {
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
        case AST_INT_LITERAL:
            return "INT_LITERAL";
        case AST_UNIT:
            return "UNIT";
        case AST_STATEMENT:
            return "STATEMENT";
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
        default:
            RuntimeError::raise("Unknown AST node type %d\n", tag);
    }
}
