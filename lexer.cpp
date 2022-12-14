#include <map>
#include <cassert>
#include <cctype>
#include <string>
#include <iostream>
#include "token.h"
#include "exceptions.h"
#include "lexer.h"

////////////////////////////////////////////////////////////////////////
// Lexer implementation
////////////////////////////////////////////////////////////////////////

Lexer::Lexer(FILE *in, const std::string &filename)
        : m_in(in), m_filename(filename), m_line(1), m_col(1), m_eof(false) {
}

Lexer::~Lexer() {
    // delete any cached lookahead tokens
    for (auto i = m_lookahead.begin(); i != m_lookahead.end(); ++i) {
        delete *i;
    }
    fclose(m_in);
}

Node *Lexer::next() {
    fill(1);
    if (m_lookahead.empty()) {
        SyntaxError::raise(get_current_loc(), "Unexpected end of input");
    }
    Node *tok = m_lookahead.front();
    m_lookahead.pop_front();
    return tok;
}

Node *Lexer::peek(int how_many) {
    // try to get as many lookahead tokens as required
    fill(how_many);

    // if there aren't enough lookahead tokens,
    // then the input ended before the token we want
    if (int(m_lookahead.size()) < how_many) {
        return nullptr;
    }

    // return the pointer to the Node representing the token
    return m_lookahead.at(how_many - 1);
}

Location Lexer::get_current_loc() const {
    return Location(m_filename, m_line, m_col);
}

// Read the next character of input, returning -1 (and setting m_eof to true)
// if the end of input has been reached.
int Lexer::read() {
    if (m_eof) {
        return -1;
    }
    int c = fgetc(m_in);
    if (c < 0) {
        m_eof = true;
    } else if (c == '\n') {
        m_col = 1;
        m_line++;
    } else {
        m_col++;
    }
    return c;
}

// "Unread" a character.  Useful for when reading a character indicates
// that the current token has ended and the next one has begun.
void Lexer::unread(int c) {
    ungetc(c, m_in);
    m_col--;
}

void Lexer::fill(int how_many) {
    assert(how_many > 0);
    while (!m_eof && int(m_lookahead.size()) < how_many) {
        Node *tok = read_token();

        if (tok != nullptr) {
            m_lookahead.push_back(tok);
        }
    }
}


Node *Lexer::read_token() {
    int c, line = -1, col = -1;

    // skip whitespace characters until a non-whitespace character is read
    for (;;) {
        line = m_line;
        col = m_col;
        c = read();
        if (c < 0 || !isspace(c)) {
            break;
        }
    }

    if (c < 0) {
        // reached end of file
        return nullptr;
    }

    std::string lexeme;
    lexeme.push_back(char(c));
    if (isalpha(c)) {
        Node *tok = read_continued_token(TOK_IDENTIFIER, lexeme, line, col, isalnum);
        std::string word = tok->get_str();
        if (word == "var") {
            tok->set_tag(TOK_VAR);
        } else if (word == "function") {
            tok->set_tag(TOK_FN);
        } else if (word == "if") {
            tok->set_tag(TOK_IF);
        } else if (word == "else") {
            tok->set_tag(TOK_ELSE);
        } else if (word == "while") {
            tok->set_tag(TOK_WHILE);
        }
        return tok;
    } else if (isdigit(c)) {
        return read_continued_token(TOK_INTEGER_LITERAL, lexeme, line, col, isdigit);
    } else {
        switch (c) {
            case '+':
                return token_create(TOK_PLUS, lexeme, line, col);
            case '-':
                return token_create(TOK_MINUS, lexeme, line, col);
            case '*':
                return token_create(TOK_TIMES, lexeme, line, col);
            case '/':
                return token_create(TOK_DIVIDE, lexeme, line, col);
            case '(':
                return token_create(TOK_LPAREN, lexeme, line, col);
            case ')':
                return token_create(TOK_RPAREN, lexeme, line, col);
            case '{':
                return token_create(TOK_LBRACE, lexeme, line, col);
            case '}':
                return token_create(TOK_RBRACE, lexeme, line, col);
            case ',':
                return token_create(TOK_COMMA, lexeme, line, col);
            case ';':
                return token_create(TOK_SEMICOLON, lexeme, line, col);
            case '=':
                return read_multi_equal(lexeme, line, col);
            case '<':
                return read_multi_less(lexeme, line, col);
            case '>':
                return read_multi_greater(lexeme, line, col);
            case '|':
                return read_continued_token(TOK_OR, lexeme, line, col, ispunct);
            case '&':
                return read_continued_token(TOK_AND, lexeme, line, col, ispunct);
            case '!':
                return read_continued_token(TOK_NOTEQUAL, lexeme, line, col, ispunct);
            case '"':
                return read_multi_string(lexeme, line, col);
            default:
                SyntaxError::raise(get_current_loc(), "Unrecognized character '%c'", c);
        }
    }
}

// Helper function to create a Node object to represent a token.
Node *Lexer::token_create(enum TokenKind kind, const std::string &lexeme, int line, int col) {
    Node *token = new Node(kind, lexeme);
    Location source_info(m_filename, line, col);
    token->set_loc(source_info);
    return token;
}

// Read the continuation of a (possibly) multi-character token, such as
// an identifier or integer literal.  pred is a pointer to a predicate
// function to determine which characters are valid continuations.
Node *
Lexer::read_continued_token(enum TokenKind kind, const std::string &lexeme_start, int line, int col, int (*pred)(int)) {
    std::string lexeme(lexeme_start);
    for (;;) {
        int c = read();
        if (c >= 0 && pred(c)) {
            // token has finished
            lexeme.push_back(char(c));
        } else {
            if (c >= 0) {
                unread(c);
            }
            return token_create(kind, lexeme, line, col);
        }
    }
}

Node *Lexer::read_multi_equal(const std::string &lexeme, int line, int col) {
    enum TokenKind kind;

    int next_c = read();
    if (next_c < 0) {
        // at eof
        kind = TOK_ASSIGN;
    } else if (next_c == '=') {
        kind = TOK_EQUAL;
    } else {
        // because we read in a token, this would prevent the next value being read correctly
        unread(next_c);
        kind = TOK_ASSIGN;
    }
    return token_create(kind, lexeme, line, col);
}

Node *Lexer::read_multi_less(const std::string &lexeme, int line, int col) {
    enum TokenKind kind;

    int next_c = read();
    if (next_c < 0) {
        // at eof
        kind = TOK_LESS;

    } else if (next_c == '=') {
        kind = TOK_LESSEQUAL;
    } else {
        unread(next_c);
        kind = TOK_LESS;
    }
    return token_create(kind, lexeme, line, col);
}

Node *Lexer::read_multi_greater(const std::string &lexeme, int line, int col) {
    enum TokenKind kind;

    int next_c = read();
    if (next_c < 0) {
        // at eof
        kind = TOK_GREATER;

    } else if (next_c == '=') {
        kind = TOK_GREATEREQUAL;
    } else {
        unread(next_c);
        kind = TOK_GREATER;
    }
    return token_create(kind, lexeme, line, col);
}

Node *Lexer::read_multi_string(const std::string &lexeme, int line, int col) {
    int next_c = read();
    std::string word;
    while (next_c != '"') {
        if (next_c == '\\') {
            int next_next = read();
            switch (next_next) {
                case '"':
                    next_c = '\"';
                    break;
                case 'n':
                    next_c = '\n';
                    break;
                case 'r':
                    next_c = '\r';
                    break;
                case 't':
                    next_c = '\t';
                    break;
                default:
                    unread(next_next);
            }
        }
        word.push_back(next_c);
        next_c = read();
    }
    return token_create(TOK_STRING, word, line, col);
}

std::string Lexer::node_tag_to_string(int tag) {
    // was used for debugging, will keep in case of future modifications to lexer
    switch (tag) {
        case TOK_FN:
            return "FUNCTION";
        case TOK_IF:
            return "IF";
        case TOK_ELSE:
            return "ELSE";
        case TOK_WHILE:
            return "WHILE";
        case TOK_IDENTIFIER:
            return "VARREF";
        case TOK_VAR:
            return "VARDEF";
        case TOK_ASSIGN:
            return "ASSIGN";
        case TOK_INTEGER_LITERAL:
            return "INT_LITERAL";
        case TOK_STRING:
            return "STRING";
        case TOK_PLUS:
            return "ADD";
        case TOK_MINUS:
            return "SUB";
        case TOK_TIMES:
            return "MULTIPLY";
        case TOK_DIVIDE:
            return "DIVIDE";
        case TOK_LPAREN:
            return "LPAREN";
        case TOK_RPAREN:
            return "RPAREN";
        case TOK_LBRACE:
            return "LBRACE";
        case TOK_RBRACE:
            return "RBRACE";
        case TOK_OR:
            return "OR";
        case TOK_AND:
            return "AND";
        case TOK_LESS:
            return "LESS";
        case TOK_LESSEQUAL:
            return "LESSEQUAL";
        case TOK_GREATER:
            return "GREATER";
        case TOK_GREATEREQUAL:
            return "GREATEREQUAL";
        case TOK_EQUAL:
            return "EQUAL";
        case TOK_NOTEQUAL:
            return "NOT";
        case TOK_SEMICOLON:
            return "SEMI";
        case TOK_COMMA:
            return "COMMA";
        default:
            return "";
    }
}
