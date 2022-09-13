#ifndef AST_H
#define AST_H

#include "treeprint.h"

// AST node tags
enum ASTKind {
  AST_ADD = 2000,
  AST_SUB,
  AST_MULTIPLY,
  AST_DIVIDE,
  AST_VARREF,
  AST_VARDEF,
  AST_INT_LITERAL,
  AST_UNIT,
  AST_STATEMENT,
  AST_ASSIGN,
  AST_AND,
  AST_OR,
  AST_LESS,
  AST_LESSEQUAL,
  AST_GREATER,
  AST_GREATEREQUAL,
  AST_EQUAL,
  AST_NOTEQUAL
};

class ASTTreePrint : public TreePrint {
public:
  ASTTreePrint();
  virtual ~ASTTreePrint();

  virtual std::string node_tag_to_string(int tag) const;
};

#endif // AST_H
