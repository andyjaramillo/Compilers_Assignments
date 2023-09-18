#ifndef TOKEN_H
#define TOKEN_H

// This header file defines the tags used for tokens (i.e., terminal
// symbols in the grammar.)

enum TokenKind {
  TOK_IDENTIFIER,
  TOK_INTEGER_LITERAL,
  TOK_PLUS,
  TOK_MINUS,
  TOK_TIMES,
  TOK_DIVIDE,
  TOK_LPAREN,
  TOK_RPAREN,
  TOK_SEMICOLON,
  TOK_VAR,
  TOK_EQUALS,
  TOK_OR,
  TOK_AND,
  TOK_LESS,
  TOK_LESSOREQUAL,
  TOK_GREATER,
  TOK_GREATEROREQUAL,
  TOK_COMPARE,
  TOK_NOTCOMPARE,
  // TODO: add members for additional kinds of tokens
};

#endif // TOKEN_H
