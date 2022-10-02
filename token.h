#ifndef TOKEN_H
#define TOKEN_H

// This header file defines the tags used for tokens (i.e., terminal
// symbols in the grammar.)

enum TokenKind {
    TOK_IDENTIFIER,
    TOK_VAR,
    TOK_ASSIGN,
    TOK_INTEGER_LITERAL,
    TOK_STRING,
    TOK_PLUS,
    TOK_MINUS,
    TOK_TIMES,
    TOK_DIVIDE,
    TOK_LPAREN,
    TOK_RPAREN,
    TOK_RBRACE,
    TOK_LBRACE,
    TOK_COMMA,
    TOK_SEMICOLON,
    TOK_FN,
    TOK_IF,
    TOK_ELSE,
    TOK_WHILE,
    // operators
    TOK_OR,
    TOK_AND,
    TOK_LESS,
    TOK_LESSEQUAL,
    TOK_GREATER,
    TOK_GREATEREQUAL,
    TOK_EQUAL,
    TOK_NOTEQUAL,
};

#endif // TOKEN_H
