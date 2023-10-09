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
  case AST_VARDEF:
    return "VARDEF";
  case AST_INT_LITERAL:
    return "INT_LITERAL";
  case AST_UNIT:
    return "UNIT";
  case AST_STATEMENT:
    return "STATEMENT";
  case AST_ASSIGNMENT:
    return "ASSIGN";
  case AST_OR:
    return "LOGICAL_OR";
  case AST_AND:
    return "LOGICAL_AND";
  case AST_LESS:
    return "LT";
  case AST_LESSOREQUAL:
    return "LTE";
  case AST_GREATER:
    return "GT";
  case AST_GREATEROREQUAL:
    return "GTE";
  case AST_COMPARE:
    return "EQ";
  case AST_NOTCOMPARE:
    return "NEQ";
  case AST_IF:
    return "IF_STATEMENT";
  case AST_ELSE:
    return "ELSE_STATEMENT";
  case AST_WHILE:
    return "WHILE_STATEMENT";
  case AST_FUNC:
    return "FUNCTION";
  case AST_PARAMETER_LIST:
    return "PARAMETER_LIST";
  case AST_STATEMENT_LIST:
    return "STATEMENT_LIST";
  case AST_FNCALL:
    return "FNCALL";
  case AST_ARGLIST:
    return "ARGLIST";
  case AST_STRING:
    return "STRING";
  // TODO: add cases for other AST node kinds
  default:
    RuntimeError::raise("Unknown AST node type %d\n", tag);
  }
}
