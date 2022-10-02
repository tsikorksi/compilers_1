#include <string>
#include <memory>
#include <iostream>
#include <cassert>
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
// Unit -> TStmt
// Unit -> TStmt Unit
// TStmt →      Stmt
// Stmt →       if ( A ) { SList }                        -- if statement
// Stmt →       if ( A ) { SList } else { SList }         -- if/else statement
// Stmt →       while ( A ) { SList }                     -- while loop
// Stmt -> A ;
// Stmt → var ident ;
// TStmt →      Func
// Func →       function ident ( OptPList ) { SList }     -- function definition
// OptPList →   PList                                     -- optional parameter list
// OptPList →   ε
// PList →      ident                                     -- nonempty parameter list
// PList →      ident , PList
// SList →      Stmt                                      -- statement list
// SList →      Stmt SList
// F →          ident ( OptArgList )                      -- function call
// OptArgList → ArgList                                   -- optional argument list
// OptArgList → ε
// ArgList →    L                                         -- nonempty argument list
// ArgList →    L , ArgList
// A    → ident = A
// A    → L
// L    → R || R
// L    → R && R
// L    → R
// R    → E < E
// R    → E <= E
// R    → E > E
// R    → E >= E
// R    → E == E
// R    → E != E
// R    → E
// E -> T E'
// E' -> + T E'
// E' -> - T E'
// E' -> epsilon
// T -> F T'
// T' -> * F T'
// T' -> / F T'
// T' -> epsilon
// F -> number
// F → string_literal
// F -> ident
// F -> ( A )




Parser2::Parser2(Lexer *lexer_to_adopt)
        : m_lexer(lexer_to_adopt), m_next(nullptr) {
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

    std::unique_ptr<Node> unit(new Node(AST_UNIT));
    for (;;) {
        unit->append_kid(parse_TStmt());
        if (m_lexer->peek() == nullptr)
            break;
    }

    return unit.release();
}

Node *Parser2::parse_TStmt() {
    //TStmt →      Func
    //TStmt →      Stmt
    std::unique_ptr<Node> s(new Node(AST_STATEMENT));

    Node *next_tok = m_lexer->peek();

    if (next_tok == nullptr) {
        SyntaxError::raise(m_lexer->get_current_loc(), "Unexpected end of input looking for statement");

    } else if (next_tok->get_tag() ==  TOK_FN) {
        //TStmt →      Func
        return parse_Func();
    }

    //TStmt →      Stmt
    return parse_Stmt();

}

Node *Parser2::parse_Stmt() {
    // Stmt ->  A ;
    // Stmt ->  var ident ;
    // Stmt ->  if ( A ) { SList }                        -- if statement
    // Stmt ->  if ( A ) { SList } else { SList }         -- if/else statement
    // Stmt ->  while ( A ) { SList }                     -- while loop

    std::unique_ptr<Node> s(new Node(AST_STATEMENT));

    Node *next_tok = m_lexer->peek();
    if (next_tok == nullptr) {
        SyntaxError::raise(m_lexer->get_current_loc(), "Unexpected end of input looking for statement");

    }

    int tag = next_tok->get_tag();

    if (tag == TOK_VAR) {
        // Stmt -> ^ var ident ;
        s->append_kid(parse_var());
        // Stmt -> var ident ^ ;
        expect_and_discard(TOK_SEMICOLON);
        return s.release();

    } else if (tag == TOK_IF || tag == TOK_WHILE) {
        // Stmt →      ^ if ( A ) { SList }                        -- if statement
        // Stmt →      ^ if ( A ) { SList } else { SList }         -- if/else statement
        // Stmt →      ^ while ( A ) { SList }                     -- while loop
        Node * ast;
        if (tag == TOK_IF) {
            // Stmt →      ^ if ( A ) { SList }
            ast = parse_if();
        } else {
            // Stmt →      ^ while ( A ) { SList }
            ast = parse_while();
        }

        // Stmt →      ctrl ^( A ) { SList }
        expect_and_discard(TOK_LPAREN);
        ast->append_kid(parse_A());
        expect_and_discard(TOK_RPAREN);

        // Stmt →      ctrl ( A ) ^{ SList }
        expect_and_discard(TOK_LBRACE);
        std::unique_ptr<Node> slist(new Node(AST_STATEMENT_LIST));
        ast->append_kid(parse_SList());
        expect_and_discard(TOK_RBRACE);

        // Could very easily allow for else statements on while loops
        next_tok = m_lexer->peek();
        if (next_tok != nullptr && next_tok->get_tag() == TOK_ELSE && ast->get_tag() == AST_IF) {
            // Stmt →       if ( A ) { SList } ^else { SList }
            expect_and_discard(TOK_ELSE);
            expect_and_discard(TOK_LBRACE);
            // generate node for content of loop

            std::unique_ptr<Node> else_slist(new Node(AST_STATEMENT_LIST));
            ast->append_kid(parse_SList());
            expect_and_discard(TOK_RBRACE);
        }
        s->append_kid(ast);
        return s.release();

    }
    // Stmt -> ^ A ;
    s->append_kid(parse_A());
    expect_and_discard(TOK_SEMICOLON);

    return s.release();
}

Node *Parser2::parse_Func() {
    // Func →       function ident ( OptPList ) { SList }     -- function definition

    Node * func_ast = parse_function();
    func_ast->append_kid(parse_ident());

    expect_and_discard(TOK_LPAREN);
    func_ast->append_kid(parse_OptPList());
    expect_and_discard(TOK_RPAREN);

    expect_and_discard(TOK_LBRACE);
    func_ast->append_kid(parse_SList());
    expect_and_discard(TOK_RBRACE);


    return func_ast;
}

Node *Parser2::parse_OptPList() {
    // OptPList →   PList                                     -- optional parameter list
    // OptPList →   ε

    std::unique_ptr<Node> s(new Node(AST_PARAMETER_LIST));

    Node *next_tok = m_lexer->peek();

    if (next_tok != nullptr && next_tok->get_tag() == TOK_IDENTIFIER) {
        // OptPList →   ^PList
        // PList starts with identifier
        return parse_PList();
    }
    // OptPList →   ^ε
    return s.release();
}


Node *Parser2::parse_PList() {
    // PList →      ident                                     -- nonempty parameter list
    // PList →      ident , PList

    std::unique_ptr<Node> opt_list(new Node(AST_PARAMETER_LIST));


    opt_list->append_kid(parse_ident());

    Node *next_tok = m_lexer->peek(1);
    while (next_tok->get_tag() == TOK_COMMA) {
        expect_and_discard(TOK_COMMA);
        opt_list->append_kid(parse_ident());
        next_tok = m_lexer->peek();
    }
    return opt_list.release();
}

Node *Parser2::parse_OptArgList() {
    // OptArgList → ArgList                                   -- optional argument list
    // OptArgList → ε

    std::unique_ptr<Node> s(new Node(AST_ARGLIST));

    Node *next_tok = m_lexer->peek(1);

    if (next_tok->get_tag() != TOK_RPAREN) {
        // OptPList →   ^PList
        // PList starts with identifier
        return parse_ArgList(s.release());
    }
    // OptPList →   ^ε
    return s.release();
}


Node *Parser2::parse_ArgList(Node *arg_list_) {
    // ArgList →    L                                         -- nonempty argument list
    // ArgList →    L , ArgList

    std::unique_ptr<Node> arg_list(arg_list_);


    arg_list->append_kid(parse_L());

    Node *next_tok = m_lexer->peek(1);
    while (next_tok->get_tag() == TOK_COMMA) {
        expect_and_discard(TOK_COMMA);
        arg_list->append_kid(parse_L());
        next_tok = m_lexer->peek();
    }
    return arg_list.release();
}


Node *Parser2::parse_SList() {
    // SList →      Stmt                                      -- statement list
    // SList →      Stmt SList
    std::unique_ptr<Node> slist(new Node(AST_STATEMENT_LIST));

    Node *next_tok = m_lexer->peek();

    // Keep searching for new segments until you hit the end of the function scope
    while (next_tok->get_tag() != TOK_RBRACE) {
        slist->append_kid(parse_Stmt());
        next_tok = m_lexer->peek();
    }
    return slist.release();
}


Node *Parser2::parse_A() {
    // A → ^ ident = A
    // A → ^ L

    Node *next_tok = m_lexer->peek(1);
    Node *next_next_tok = m_lexer->peek(2);
    if (next_tok == nullptr || next_next_tok == nullptr) {
        Parser2::error_at_current_loc("Unexpected end of input");
    }
    int next_tok_tag = next_tok->get_tag();
    int next_next_tok_tag = next_next_tok->get_tag();
    if (next_tok_tag == TOK_IDENTIFIER && next_next_tok_tag == TOK_ASSIGN) {
        // A → ^ ident = A
        return parse_assign();
    } else {
        // A → ^ L
        return parse_L();
    }
}

Node *Parser2::parse_L() {
    //L    → R || R
    //L    → R && R
    //L    → R

    Node *lhs = parse_R();

    Node *next_tok = m_lexer->peek();

    if (next_tok != nullptr) {
        if (next_tok->get_tag() == TOK_AND || next_tok->get_tag() == TOK_OR) {
            //L    → R ^|| R
            //L    → R ^&& R

            int tag = next_tok->get_tag();

            int ast_tag = tok_to_ast(static_cast<TokenKind>(tag));
            std::unique_ptr<Node> op(expect(static_cast<enum TokenKind>(tag)));
            //L    → R ||^ R
            //L    → R &&^ R
            Node *rhs = parse_R();
            op->append_kid(lhs);
            op->append_kid(rhs);
            op->set_str(next_tok->get_str());
            op->set_loc(next_tok->get_loc());
            op->set_tag(ast_tag);

            return op.release();
        }

    }

    //L    → R
    return lhs;

}

Node *Parser2::parse_R() {
    //R    → E < E
    //R    → E <= E
    //R    → E > E
    //R    → E >= E
    //R    → E == E
    //R    → E != E
    //R    → E

    //R    → ^E op E
    Node *lhs = parse_E();

    Node *next_tok = m_lexer->peek(1);

    if (next_tok == nullptr) {
        Parser2::error_at_current_loc("Unexpected end of input");
    }
    if (next_tok->get_tag() < 27 && next_tok->get_tag() > 18) {
        //R    → E ^op E
        std::unique_ptr<Node> tok(expect(static_cast<enum TokenKind>(next_tok->get_tag())));
        int ast_tag = tok_to_ast(static_cast<TokenKind>(next_tok->get_tag()));
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
    return lhs;
}


Node *Parser2::parse_E() {
    // E -> ^ T E'

    // Get the AST corresponding to the term (T)

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


    std::unique_ptr<Node> ast(ast_);

    // peek at next token
    Node *next_tok = m_lexer->peek();
    if (next_tok != nullptr) {
        int next_tok_tag = next_tok->get_tag();
        if (next_tok_tag == TOK_PLUS || next_tok_tag == TOK_MINUS) {
            // E' -> ^ + T E'
            // E' -> ^ - T E'
            std::unique_ptr<Node> op(expect(static_cast<enum TokenKind>(next_tok_tag)));

            // build AST for next term, incorporate into current AST
            Node *term_ast = parse_T();
            // E' ->  - T^ E'
            ast.reset(new Node(tok_to_ast(static_cast<TokenKind>(next_tok_tag)), {ast.release(), term_ast}));

            // copy source information from operator node
            ast->set_loc(op->get_loc());

            // continue recursively
            return parse_EPrime(ast.release());
        }
    }
    // E' -> ^ epsilon
    // No more additive operators, so just return the completed AST
    return ast.release();
}

Node *Parser2::parse_T() {
    // T -> F T'

    // Parse primary expression
    Node *ast = parse_F();

    // Recursively continue the multiplicative expression
    return parse_TPrime(ast);
}

Node *Parser2::parse_TPrime(Node *ast_) {
    // T' -> ^ * F T'
    // T' -> ^ / F T'
    // T' -> ^ epsilon

    std::unique_ptr<Node> ast(ast_);

    // peek at next token
    Node *next_tok = m_lexer->peek(1);
    if (next_tok != nullptr) {
        int next_tok_tag = next_tok->get_tag();
        if (next_tok_tag == TOK_TIMES || next_tok_tag == TOK_DIVIDE) {
            // T' -> ^ * F T'
            // T' -> ^ / F T'

            std::unique_ptr<Node> op(expect(static_cast<enum TokenKind>(next_tok_tag)));

            // build AST for next primary expression, incorporate into current AST
            Node *primary_ast = parse_F();
            ast.reset(new Node(tok_to_ast(static_cast<TokenKind>(next_tok_tag)), {ast.release(), primary_ast}));

            // copy source information from operator node
            ast->set_loc(op->get_loc());

            // continue recursively
            return parse_TPrime(ast.release());
        }
    }

    // T' -> ^ epsilon
    // No more multiplicative operators, so just return the completed AST

    return ast.release();
}

Node *Parser2::parse_F() {
    // F -> ^ number
    // F -> ^ ident
    // F -> ^ ident ( OptArgList )     -- function call
    // F -> ^ ( A )
    // F -> string_literal

    Node *next_tok = m_lexer->peek();
    Node *next_next_tok = m_lexer->peek(2);
    if (next_tok == nullptr) {
        error_at_current_loc("Unexpected end of input looking for primary expression");
    }

    int tag = next_tok->get_tag();
    if (tag == TOK_INTEGER_LITERAL || tag == TOK_IDENTIFIER || tag == TOK_STRING) {
        // F -> ^ number
        // F -> ^ ident
        std::unique_ptr<Node> tok(expect(static_cast<enum TokenKind>(tag)));
        std::unique_ptr<Node> ast(new Node(tok_to_ast(static_cast<TokenKind>(tag))));
        ast->set_str(tok->get_str());
        ast->set_loc(tok->get_loc());
        if (next_next_tok->get_tag() == TOK_LPAREN) {
            // F -> ident ^ ( OptArgList )     -- function call
            expect_and_discard(TOK_LPAREN);
            ast->append_kid(parse_OptArgList());
            expect_and_discard(TOK_RPAREN);
            return ast.release();
        }
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

    Node *lhs = parse_ident();
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

    std::unique_ptr<Node> tok(expect(static_cast<enum TokenKind>(TOK_VAR)));
    std::unique_ptr<Node> ast(new Node(AST_VARDEF));
    ast->set_str(tok->get_str());
    ast->set_loc(tok->get_loc());
    ast->append_kid(parse_ident());

    return ast.release();
}


Node *Parser2::parse_ident() {
    // STMT -> var ^ ident;

    std::unique_ptr<Node> tok(expect(static_cast<enum TokenKind>(TOK_IDENTIFIER)));
    std::unique_ptr<Node> ast(new Node(AST_VARREF));
    ast->set_str(tok->get_str());
    ast->set_loc(tok->get_loc());
    return ast.release();

}

Node *Parser2::parse_if() {
    // Read and create an IF statement
    std::unique_ptr<Node> tok(expect(static_cast<enum TokenKind>(TOK_IF)));
    std::unique_ptr<Node> ast(new Node(AST_IF));
    ast->set_str(tok->get_str());
    ast->set_loc(tok->get_loc());
    return ast.release();
}


Node *Parser2::parse_while() {
    // read and create While Statement
    std::unique_ptr<Node> tok(expect(static_cast<enum TokenKind>(TOK_WHILE)));
    std::unique_ptr<Node> ast(new Node(AST_WHILE));
    ast->set_str(tok->get_str());
    ast->set_loc(tok->get_loc());
    return ast.release();
}

Node *Parser2::parse_function() {
    std::unique_ptr<Node> tok(expect(static_cast<enum TokenKind>(TOK_FN)));
    std::unique_ptr<Node> ast(new Node(AST_FUNCTION));
    ast->set_loc(tok->get_loc());
    return ast.release();
}


Node *Parser2::expect(enum TokenKind tok_kind) {
    std::unique_ptr<Node> next_terminal(m_lexer->next());
    if (next_terminal == nullptr) {
        SyntaxError::raise(next_terminal->get_loc(), "Unexpected end of input, wanted %u", tok_kind);
    }
    else if (next_terminal->get_tag() != tok_kind) {
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

ASTKind Parser2::tok_to_ast(TokenKind tag) {
    switch (tag) {
        case TOK_FN:
            return AST_FUNCTION;
        case TOK_IF:
            return AST_IF;
        case TOK_WHILE:
            return AST_WHILE;
        case TOK_ELSE:
            return AST_ELSE;
        case TOK_IDENTIFIER:
            return AST_VARREF;
        case TOK_VAR:
            return AST_VARDEF;
        case TOK_ASSIGN:
            return AST_ASSIGN;
        case TOK_INTEGER_LITERAL:
            return AST_INT_LITERAL;
        case TOK_STRING:
            return AST_STRING;
        case TOK_PLUS:
            return AST_ADD;
        case TOK_MINUS:
            return AST_SUB;
        case TOK_TIMES:
            return AST_MULTIPLY;
        case TOK_DIVIDE:
            return AST_DIVIDE;
        case TOK_OR:
            return AST_OR;
        case TOK_AND:
            return AST_AND;
        case TOK_LESS:
            return AST_LESS;
        case TOK_LESSEQUAL:
            return AST_LESSEQUAL;
        case TOK_GREATER:
            return AST_GREATER;
        case TOK_GREATEREQUAL:
            return AST_GREATEREQUAL;
        case TOK_EQUAL:
            return AST_EQUAL;
        case TOK_NOTEQUAL:
            return AST_NOTEQUAL;
        case TOK_LPAREN:
        case TOK_RPAREN:
        case TOK_SEMICOLON:
        case TOK_LBRACE:
        case TOK_RBRACE:
        case TOK_COMMA:
        default:
            break;
    }
    Parser2::error_at_current_loc("Token failed to convert to valid AST Tag");
    assert(0);
}




