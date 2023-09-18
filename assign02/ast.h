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
  AST_ASSIGNMENT,
  AST_OR,
  AST_AND,
  AST_LESS,
  AST_LESSOREQUAL,
  AST_GREATER,
  AST_GREATEROREQUAL,
  AST_COMPARE,
  AST_NOTCOMPARE,
  // TODO: add members for other AST node kinds
};

class ASTTreePrint : public TreePrint {
public:
  ASTTreePrint();
  virtual ~ASTTreePrint();

  virtual std::string node_tag_to_string(int tag) const;
};

#endif // AST_H
