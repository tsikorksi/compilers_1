#include <string>
#include <memory>
#include <iostream>
#include "token.h"
#include "ast.h"
#include "exceptions.h"
#include "parser2.h"

////////////////////////////////////////////////////////////////////////
// Parser2 implementation
// This version of the parser builds an AST directly,
// rather than first building a parse tree.
////////////////////////////////////////////////////////////////////////

// This is the grammar (Unit is the start symbol):
//
// Unit -> Stmt
// Unit -> Stmt Unit
// Stmt -> E ;
// E -> T E'
// E' -> + T E'
// E' -> - T E'
// E' -> epsilon
// T -> F T'
// T' -> * F T'
// T' -> / F T'
// T' -> epsilon
// F -> number
// F -> ident
// F -> ( E )
// c = (a * 4 < 55) && (b == 16 || b >= 9 + 13);


Parser2::Parser2(Lexer *lexer_to_adopt)
        : m_lexer(lexer_to_adopt)
        , m_next(nullptr) {
}

Parser2::~Parser2() {
    delete m_lexer;
}

Node *Parser2::parse() {
    return parse_Unit();
}
Node *Parser2::parse_Unit() {
    // note that this function produces a "flattened" representation
    // of the unit

    std::cout << "UNIT" << std::endl;
    std::unique_ptr<Node> unit(new Node(AST_UNIT));
    for (;;) {
        unit->append_kid(parse_Stmt());
        if (m_lexer->peek() == nullptr)
            break;
    }

    return unit.release();
}

Node *Parser2::parse_Stmt() {
    // Stmt -> ^ E ;
    // Stmt -> ^ var ident ;

    std::cout << "STMT" << std::endl;
    std::unique_ptr<Node> s(new Node(AST_STATEMENT));

    Node *next_tok = m_lexer->peek();

    if (next_tok == nullptr) {
        SyntaxError::raise(m_lexer->get_current_loc(), "Unexpected end of input looking for statement");

    } else if (next_tok->get_tag() == TOK_VAR) {
        // Stmt -> ^ var ident ;
        s->append_kid(parse_var());
        expect_and_discard(TOK_SEMICOLON);
        return s.release();

    }
    // Stmt -> ^ E ;
    s->append_kid(parse_A());
    expect_and_discard(TOK_SEMICOLON);

    return s.release();
}


Node *Parser2::parse_A() {
    // A → ^ ident = A
    // A → ^ L

    std::cout << "A" << std::endl;
    Node *next_tok = m_lexer->peek(1);
    Node *next_next_tok = m_lexer->peek(2);
    int next_tok_tag = next_tok->get_tag();
    int next_next_tok_tag = next_next_tok->get_tag();
    if (next_tok_tag == TOK_IDENTIFIER && next_next_tok_tag == TOK_ASSIGN){
        return parse_assign();
    } else {
        return parse_L();
    }
}

Node *Parser2::parse_L(){
    //L    → R || R
    //L    → R && R
    //L    → R
    std::cout << "L" << std::endl;

    Node *next_tok = m_lexer->peek();

    if (next_tok != nullptr) {
        if (next_tok->get_tag() == TOK_AND || next_tok->get_tag() == TOK_OR) {
            //L    → R || R
            //L    → R && R
            Node *lhs = parse_R();

            int tag = next_tok->get_tag();

            int ast_tag = tag == TOK_AND ? AST_AND : AST_OR;
            std::unique_ptr<Node> op(expect(static_cast<enum TokenKind>(ast_tag)));
            Node *rhs = parse_R();
            op->append_kid(lhs);
            op->append_kid(rhs);
            op->set_str(next_tok->get_str());
            op->set_loc(next_tok->get_loc());
        }

    }

    //L    → R
    return parse_R();

}

Node *Parser2::parse_R() {
    //R    → E < E
    //R    → E <= E
    //R    → E > E
    //R    → E >= E
    //R    → E == E
    //R    → E != E
    //R    → E

    std::cout << "R" << std::endl;
    Node *next_tok = m_lexer->peek(2);

    std::cout << next_tok->get_tag() << std::endl;

    if (next_tok->get_tag() < 18 && next_tok->get_tag() > 11) {
        std::cout << "R op R" << std::endl;
        //R    → ^E op E
        Node *lhs = parse_E();
        //R    → E ^op E
        std::unique_ptr<Node> tok(expect(static_cast<enum TokenKind>(next_tok->get_tag())));
        int ast_tag = next_tok->get_tag() + 2000;
        std::unique_ptr<Node> ast(new Node(ast_tag));
        //R    → E op ^E
        Node *rhs = parse_E();
        ast->append_kid(lhs);
        ast->append_kid(rhs);
        ast->set_str(tok->get_str());
        ast->set_loc(tok->get_loc());
        return ast.release();
    }

    //R    → E
    return parse_E();
}


Node *Parser2::parse_E() {
    // E -> ^ T E'

    // Get the AST corresponding to the term (T)

    std::cout << "E" << std::endl;
    Node *ast = parse_T();

    // Recursively continue the additive expression
    return parse_EPrime(ast);
}

// This function is passed the "current" portion of the AST
// that has been built so far for the additive expression.
Node *Parser2::parse_EPrime(Node *ast_) {
    // E' -> ^ + T E'
    // E' -> ^ - T E'
    // E' -> ^ epsilon

    std::cout << "E'" << std::endl;

    std::unique_ptr<Node> ast(ast_);

    // peek at next token
    Node *next_tok = m_lexer->peek();
    if (next_tok != nullptr) {
        int next_tok_tag = next_tok->get_tag();
        if (next_tok_tag == TOK_PLUS || next_tok_tag == TOK_MINUS)  {
            // E' -> ^ + T E'
            // E' -> ^ - T E'
            std::unique_ptr<Node> op(expect(static_cast<enum TokenKind>(next_tok_tag)));

            // build AST for next term, incorporate into current AST
            Node *term_ast = parse_T();
            ast.reset(new Node(next_tok_tag == TOK_PLUS ? AST_ADD : AST_SUB, {ast.release(), term_ast}));

            // copy source information from operator node
            ast->set_loc(op->get_loc());

            // continue recursively
            return parse_EPrime(ast.release());
        }
    }
    std::cout << "epsilon" << std::endl;
    // E' -> ^ epsilon
    // No more additive operators, so just return the completed AST
    return ast.release();
}

Node *Parser2::parse_T() {
    // T -> F T'

    // Parse primary expression
    std::cout << "T" << std::endl;
    Node *ast = parse_F();

    // Recursively continue the multiplicative expression
    return parse_TPrime(ast);
}

Node *Parser2::parse_TPrime(Node *ast_) {
    // T' -> ^ * F T'
    // T' -> ^ / F T'
    // T' -> ^ epsilon

    std::cout << "T'" << std::endl;
    std::unique_ptr<Node> ast(ast_);

    // peek at next token
    // Lexer reads and creates tokens for < and 55, but why?
    Node *next_tok = m_lexer->peek(1);
    //std::cout << next_tok->get_tag() << std::endl;
    if (next_tok != nullptr) {
        int next_tok_tag = next_tok->get_tag();
        if (next_tok_tag == TOK_TIMES || next_tok_tag == TOK_DIVIDE)  {
            // T' -> ^ * F T'
            // T' -> ^ / F T'

            std::unique_ptr<Node> op(expect(static_cast<enum TokenKind>(next_tok_tag)));

            // build AST for next primary expression, incorporate into current AST
            Node *primary_ast = parse_F();
            ast.reset(new Node(next_tok_tag == TOK_TIMES ? AST_MULTIPLY : AST_DIVIDE, {ast.release(), primary_ast}));

            // copy source information from operator node
            ast->set_loc(op->get_loc());

            // continue recursively
            return parse_TPrime(ast.release());
        }
    }

    std::cout << "epsilon" << std::endl;
    // T' -> ^ epsilon
    // No more multiplicative operators, so just return the completed AST

    return ast.release();
}

Node *Parser2::parse_F() {
    // F -> ^ number
    // F -> ^ ident
    // F -> ^ ( A )

    std::cout << "F" << std::endl;
    Node *next_tok = m_lexer->peek();
    if (next_tok == nullptr) {
        error_at_current_loc("Unexpected end of input looking for primary expression");
    }

    int tag = next_tok->get_tag();
    if (tag == TOK_INTEGER_LITERAL || tag == TOK_IDENTIFIER) {
        // F -> ^ number
        // F -> ^ ident
        std::unique_ptr<Node> tok(expect(static_cast<enum TokenKind>(tag)));
        int ast_tag = tag == TOK_INTEGER_LITERAL ? AST_INT_LITERAL : AST_VARREF;
        std::unique_ptr<Node> ast(new Node(ast_tag));
        ast->set_str(tok->get_str());
        ast->set_loc(tok->get_loc());
        return ast.release();
    } else if (tag == TOK_LPAREN) {
        // F -> ^ ( A )
        expect_and_discard(TOK_LPAREN);
        std::unique_ptr<Node> ast(parse_A());
        expect_and_discard(TOK_RPAREN);
        return ast.release();
    } else {
        SyntaxError::raise(next_tok->get_loc(), "Invalid primary expression");
    }
}

Node *Parser2::parse_assign() {
    // A  → ^ ident = A

    std::cout << "assign" << std::endl;
    Node *lhs = parse_ident();

    //std::cout << m_lexer->peek()->get_tag() << std::endl;
    //std::cout << "bang" << std::endl;

    expect_and_discard(TOK_ASSIGN);
    // A    → ident = ^ A

    Node *rhs = parse_A();

    std::unique_ptr<Node> ast(new Node(AST_ASSIGN));
    ast->set_loc(lhs->get_loc());
    ast->set_str("=");
    ast->append_kid(lhs);
    ast->append_kid(rhs);
    return ast.release();
}

Node *Parser2::parse_var() {
    // STMT -> ^ var ident;

    std::cout << "variable" << std::endl;
    expect_and_discard(TOK_VAR);

    return parse_ident();
}


Node *Parser2::parse_ident() {
    // STMT -> var ^ ident;

    std::cout << "ident" << std::endl;
    std::unique_ptr<Node> tok(expect(static_cast<enum TokenKind>(TOK_IDENTIFIER)));
    std::unique_ptr<Node> ast(new Node(AST_VARREF));
    ast->set_str(tok->get_str());
    ast->set_loc(tok->get_loc());
    return ast.release();

}

Node *Parser2::expect(enum TokenKind tok_kind) {
    std::unique_ptr<Node> next_terminal(m_lexer->next());
    std::cout << "Parser consumes: " << next_terminal->get_tag() << ", Expects: " << tok_kind << std::endl;
    if (next_terminal->get_tag() != tok_kind) {
        SyntaxError::raise(next_terminal->get_loc(), "Unexpected token '%s'", next_terminal->get_str().c_str());
    }

    return next_terminal.release();
}

void Parser2::expect_and_discard(enum TokenKind tok_kind) {
    Node *tok = expect(tok_kind);
    delete tok;
}

void Parser2::error_at_current_loc(const std::string &msg) {
    SyntaxError::raise(m_lexer->get_current_loc(), "%s", msg.c_str());
}

