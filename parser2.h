#ifndef PARSER2_H
#define PARSER2_H

#include "lexer.h"
#include "node.h"
#include "ast.h"

class Parser2 {
private:
    Lexer *m_lexer;
    Node *m_next;

public:
    Parser2(Lexer *lexer_to_adopt);
    ~Parser2();

    Node *parse();

private:
    // Parse functions for nonterminal grammar symbols
    Node *parse_Unit();
    Node *parse_Stmt();
    Node *parse_TStmt();
    Node *parse_E();
    Node *parse_EPrime(Node *ast);
    Node *parse_T();
    Node *parse_TPrime(Node *ast);
    Node *parse_F();
    Node *parse_Func();


    // Consume a specific token, wrapping it in a Node
    Node *expect(enum TokenKind tok_kind);

    // Consume a specific token and discard it
    void expect_and_discard(enum TokenKind tok_kind);

    // Report an error at current lexer position
    void error_at_current_loc(const std::string &msg);

    // Parse functions for Math

    Node *parse_L();
    Node *parse_A();
    Node *parse_R();

    // Parse functions for terminals

    Node *parse_ident();
    Node *parse_var();
    Node *parse_if();
    Node *parse_while();
    Node *parse_assign();


    // Parse functions for Lists

    Node *parse_SList(Node *ast);
    Node *parse_OptPList();
    Node *parse_PList(Node *pNode);


    // Helper function
    ASTKind tok_to_ast(TokenKind tag);


    Node *parse_OptArgList();
    Node *parse_ArgList(Node *arg_list_);

    Node *parse_function();

    Node *parse_SList();
};

#endif // PARSER2_H
