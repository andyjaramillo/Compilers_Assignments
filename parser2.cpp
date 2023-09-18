#include <cassert>
#include <map>
#include <string>
#include <memory>
#include "lexer.h"
#include "node.h"
#include "token.h"
#include "ast.h"
#include "exceptions.h"
#include "parser2.h"
#include <iostream>
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

  std::unique_ptr<Node> unit(new Node(AST_UNIT));
  for (;;) {
    unit->append_kid(parse_Stmt());
    if (m_lexer->peek() == nullptr)
      break;
  }

  return unit.release();
}

Node *Parser2::parse_Stmt() {
  // Stmt -> ^ A ;

  std::unique_ptr<Node> s(new Node(AST_STATEMENT));
  Node *next_tok = m_lexer->peek();

  if (next_tok == nullptr) {
    SyntaxError::raise(m_lexer->get_current_loc(), "Unexpected end of input looking for statement");
  }
  int tag = next_tok->get_tag();
  if(tag == TOK_VAR) {
    std::unique_ptr<Node> var(expect(static_cast<enum TokenKind>(tag)));
    std::unique_ptr<Node> ast(expect(static_cast<enum TokenKind>(TOK_IDENTIFIER)));
    var->set_tag(AST_VARDEF);
    var->set_str("");
     ast->set_tag(AST_VARREF);
    var->append_kid(ast.release());
    s->append_kid(var.release());
  } else {
    s->append_kid(parse_A());
  }
  expect_and_discard(TOK_SEMICOLON);

  return s.release();
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
  Node *next_tok = m_lexer->peek();
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

  // T' -> ^ epsilon
  // No more multiplicative operators, so just return the completed AST
  return ast.release();
}

Node *Parser2::parse_F() {

  Node *next_tok = m_lexer->peek();
  if (next_tok == nullptr) {
    error_at_current_loc("Unexpected end of input looking for primary expression_F");
  }

  int tag = next_tok->get_tag();
  if (tag == TOK_INTEGER_LITERAL || tag == TOK_IDENTIFIER) {
    // F -> ^ number
    // F -> ^ ident
    std::unique_ptr<Node> tok(expect(static_cast<enum TokenKind>(tag)));
    assert(tok->get_str().length() != 0);
    int ast_tag = tag == TOK_INTEGER_LITERAL ? AST_INT_LITERAL : AST_VARREF;
    std::unique_ptr<Node> ast(new Node(ast_tag));
    ast->set_str(tok->get_str());
    ast->set_loc(tok->get_loc());
    return ast.release();
  } else if (tag == TOK_LPAREN) {
    expect_and_discard(TOK_LPAREN);
    std::unique_ptr<Node> ast(parse_A());
    expect_and_discard(TOK_RPAREN);
    return ast.release();
  } else {
    SyntaxError::raise(next_tok->get_loc(), "parse_F error");
  }
}

Node *Parser2::parse_A() {
  Node *next_tok = m_lexer->peek();
  Node *next_next_tok = m_lexer->peek(2);
  if (next_tok == nullptr) {
    error_at_current_loc(
        "Unexpected end of input looking for primary expression_A");
  }

  if (next_tok->get_tag() != TOK_IDENTIFIER ||(next_tok->get_tag() == TOK_IDENTIFIER &&
 (next_next_tok == nullptr || next_next_tok->get_tag() != TOK_EQUALS))) {

    return parse_L();
  } else if (next_next_tok != nullptr && next_tok->get_tag() == TOK_IDENTIFIER) {
    std::unique_ptr<Node> parseIdentifier(
        expect(static_cast<enum TokenKind>(TOK_IDENTIFIER)));
    parseIdentifier->set_tag(AST_VARREF);
    expect_and_discard(static_cast<enum TokenKind>(TOK_EQUALS));
    std::unique_ptr<Node> secondParseA(parse_A());
    std::unique_ptr<Node> ast(new Node(
        AST_ASSIGNMENT, {parseIdentifier.release(), secondParseA.release()}));
    return ast.release();
  } else {
    SyntaxError::raise(next_tok->get_loc(), "parse_A error");
  }
}

Node *Parser2::parse_L() {
  std::unique_ptr<Node> firstR(parse_R());
  Node *operation = m_lexer->peek();

  if (operation == nullptr ||
      (operation != nullptr && ((operation->get_tag() != TOK_AND) &&
                                (operation->get_tag() != TOK_OR)))) {

    return firstR.release();
  } else if ((operation->get_tag() == TOK_AND) || (operation->get_tag() == TOK_OR)) {
    int next_next_tag = operation->get_tag();
    std::unique_ptr<Node> op(expect(static_cast<enum TokenKind>(next_next_tag)));
    Node *secondR = parse_R();
    std::unique_ptr<Node> ast(new Node((operation->get_tag() == TOK_AND) ? AST_AND : AST_OR,{firstR.release(), secondR}));
    return ast.release();
  } else {
    SyntaxError::raise(firstR->get_loc(), "parse_L error");
  }
}

Node *Parser2::parse_R() {

  std::unique_ptr<Node> firstR(parse_E());
  Node *next_tok = m_lexer->peek();
  if (next_tok == nullptr ||
      (next_tok != nullptr && (next_tok->get_tag() != TOK_LESS) &&
       (next_tok->get_tag() != TOK_GREATER) &&
       (next_tok->get_tag() != TOK_GREATEROREQUAL) &&
       (next_tok->get_tag() != TOK_LESSOREQUAL) &&
       (next_tok->get_tag() != TOK_COMPARE) &&
       (next_tok->get_tag() != TOK_NOTCOMPARE))) {
    return firstR.release();
  }
  int operation_tag = next_tok->get_tag();
  if ((operation_tag == TOK_LESS) || (operation_tag == TOK_GREATER) ||
      (operation_tag == TOK_GREATEROREQUAL) ||
      (operation_tag == TOK_LESSOREQUAL) || (operation_tag == TOK_COMPARE) ||
      (operation_tag == TOK_NOTCOMPARE)) {
    std::unique_ptr<Node> op(
        expect(static_cast<enum TokenKind>(operation_tag)));
    std::unique_ptr<Node> second_ast_R(parse_E());
    switch (operation_tag) {
    case TOK_LESS: {
      std::unique_ptr<Node> ast(
          new Node(AST_LESS, {firstR.release(), second_ast_R.release()}));
      return ast.release();
    }
    case TOK_GREATER: {
      std::unique_ptr<Node> ast(
          new Node(AST_GREATER, {firstR.release(), second_ast_R.release()}));
      return ast.release();
    }
    case TOK_LESSOREQUAL: {
      std::unique_ptr<Node> ast(new Node(
          AST_LESSOREQUAL, {firstR.release(), second_ast_R.release()}));
      return ast.release();
    }
    case TOK_GREATEROREQUAL: {
      std::unique_ptr<Node> ast(new Node(
          AST_GREATEROREQUAL, {firstR.release(), second_ast_R.release()}));
      return ast.release();
    }
    case TOK_COMPARE: {
      std::unique_ptr<Node> ast(
          new Node(AST_COMPARE, {firstR.release(), second_ast_R.release()}));
      return ast.release();
    }
    case TOK_NOTCOMPARE: {
      std::unique_ptr<Node> ast(
          new Node(AST_NOTCOMPARE, {firstR.release(), second_ast_R.release()}));
      return ast.release();
    }
    default:
      RuntimeError::raise("Unknown AST node type %d\n", next_tok->get_tag());
    }

  } else {
    SyntaxError::raise(next_tok->get_loc(), "parse_R error");
  }
}

Node *Parser2::expect(enum TokenKind tok_kind) {
  std::unique_ptr<Node> next_terminal(m_lexer->next());
  if (next_terminal->get_tag() != tok_kind) {
    SyntaxError::raise(next_terminal->get_loc(), "Unexpected token '%s'",
                       next_terminal->get_str().c_str());
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
