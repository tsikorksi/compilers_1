#ifndef LEXER_H
#define LEXER_H

#include <deque>
#include <cstdio>
#include "token.h"
#include "node.h"

class Lexer {
private:
    FILE *m_in;
    std::deque<Node *> m_lookahead;
    std::string m_filename;
    int m_line, m_col;
    bool m_eof;

public:
    Lexer(FILE *in, const std::string &filename);
    ~Lexer();

    // Consume the next token.
    // Throws SyntaxError if the input ends before
    // one token can be read.
    Node *next();

    // Look ahead and return a pointer to a future token
    // without consuming it. The how_far parameter indicates
    // how many tokens to look ahead (1 means return the
    // next token, 2 means the token after the next token,
    // etc.)
    Node *peek(int how_far = 1);

    Location get_current_loc() const;
    static std::string node_tag_to_string(int tag);

private:
    int read();
    void unread(int c);
    void fill(int how_many);
    Node *read_token();
    Node *token_create(enum TokenKind kind, const std::string &lexeme, int line, int col);
    Node *read_continued_token(enum TokenKind kind, const std::string &lexeme_start, int line, int col, int (*pred)(int));
    Node *read_multi_equal(const std::string &lexeme, int line, int col);

    Node *read_multi_less(const std::string &lexeme, int line, int col);

    Node *read_multi_greater(const std::string &lexeme, int line, int col);

    Node *read_multi_string(const std::string &lexeme, int line, int col);
};

#endif // LEXER_H
