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

  //These are the new productions. TStmt stands for top level
  //Unit → TStmt
  //Unit → TStmt Unit
  std::unique_ptr<Node> unit(new Node(AST_UNIT));
  for (;;) {
    unit->append_kid(parse_TStmt());
    if (m_lexer->peek() == nullptr)
      break;
  }

  return unit.release();
}

Node *Parser2::parse_TStmt() {
  //TODO TStmt →      Stmt
  //TODO TStmt →      Func
  Node* next_tok = m_lexer->peek();

  if(next_tok->get_tag() == TOK_FUNCTION){
    //TStmt →     ^ Func
    std::unique_ptr<Node> ast(parse_Func());
    return ast.release();
  } 
  //TODO Check if this needs to be a else if with an error or not
  else {
     std::unique_ptr<Node> ast(parse_Stmt());
    return ast.release();
  }

}



Node *Parser2::parse_Stmt() {
  // Stmt -> ^ A ;
  //TODO Stmt →       if ( A ) { SList }                        -- if statement
  //TODO Stmt →       if ( A ) { SList } else { SList }         -- if/else statement 
  //TODO Stmt →       while ( A ) { SList }                     -- while loop
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
  } else if(tag == TOK_IF){
    //if ( A ) { SList } 
    //if ( A ) { SList } else { SList } 
    std::unique_ptr<Node> if_tok(expect(static_cast<enum TokenKind>(TOK_IF)));
    if_tok->set_tag(AST_IF);
    assert(m_lexer->peek()->get_tag() == TOK_LPAREN);
    expect_and_discard(TOK_LPAREN);
    std::unique_ptr<Node> a_parsed(parse_A());
     if_tok->append_kid(a_parsed.release());
    expect_and_discard(TOK_RPAREN);
    expect_and_discard(TOK_LEFT_BRACKET);
    if(m_lexer->peek()->get_tag() != TOK_RIGHT_BRACKET){
      //there is something in the if statement
       Node *  statementList = new Node(AST_STATEMENT_LIST);
      std::unique_ptr<Node> sList_parse(parse_SList(statementList));
          if_tok->append_kid(sList_parse.release());
    }
    expect_and_discard(TOK_RIGHT_BRACKET);
    s->append_kid(if_tok.release());
    if(m_lexer->peek() != nullptr && m_lexer->peek()->get_tag() == TOK_ELSE) {
       std::unique_ptr<Node> else_tok(expect(static_cast<enum TokenKind>(TOK_ELSE)));
       else_tok->set_tag(AST_ELSE);
      expect_and_discard(TOK_LEFT_BRACKET);
       if(m_lexer->peek()->get_tag() != TOK_RIGHT_BRACKET){
      //there is something in the if statement
         Node *  elseStatementList = new Node(AST_STATEMENT_LIST);
         std::unique_ptr<Node> sList_parse(parse_SList(elseStatementList));
          else_tok->append_kid(sList_parse.release());
      }
      expect_and_discard(TOK_RIGHT_BRACKET);
      s->append_kid(else_tok.release());
      return s.release();
    } else {
    return s.release();
    }
  } else if(tag == TOK_WHILE){
    //Stmt →       while  ( A ) { SList }                     -- while loop
    std::unique_ptr<Node> while_tok(expect(static_cast<enum TokenKind>(TOK_WHILE)));
    while_tok->set_tag(AST_WHILE);
    expect_and_discard(TOK_LPAREN);
    std::unique_ptr<Node> parameterList(new Node(AST_PARAMETER_LIST));
    std::unique_ptr<Node> a_parsed(parse_A());
    parameterList->append_kid(a_parsed.release());
    while_tok->append_kid(parameterList.release());
    expect_and_discard(TOK_RPAREN);
    expect_and_discard(TOK_LEFT_BRACKET);
    if(m_lexer->peek()->get_tag() != TOK_RIGHT_BRACKET){
       Node *  statementList = new Node(AST_STATEMENT_LIST);
    std::unique_ptr<Node> sList_parse(parse_SList(statementList));
        while_tok->append_kid(sList_parse.release());
    }
    expect_and_discard(TOK_RIGHT_BRACKET);
   s->append_kid(while_tok.release());
    return s.release();

  } else {
    //No conditional statements
    s->append_kid(parse_A());
  }
  if(m_lexer->peek()->get_tag() != TOK_SEMICOLON){
    SyntaxError::raise(m_lexer->get_current_loc(), "Missing Semicolon");
  }
  expect_and_discard(TOK_SEMICOLON);
  return s.release();
}

 Node *Parser2::parse_Func() {
    //TODO Func →       function ident ( OptPList ) { SList }     -- function definition
    //TODO Check if this needs to be new node
    Node * s(expect(static_cast<enum TokenKind>(TOK_FUNCTION)));
    s->set_tag(AST_FUNC);
    s->set_str("");
    //Func →       function  ^ ident ( OptPList ) { SList }
    std::unique_ptr<Node> ident(expect(static_cast<enum TokenKind>(TOK_IDENTIFIER)));
     std::unique_ptr<Node> identifier_Node(new Node(AST_VARREF));
     identifier_Node->set_str(ident->get_str());
     identifier_Node->set_loc(ident->get_loc());
        s->append_kid(identifier_Node.release());
    //Func →       function ident ^ ( OptPList ) { SList }
    expect_and_discard(TOK_LPAREN);
    if(m_lexer->peek()->get_tag() != TOK_RPAREN){
       Node *  optPList = new Node(AST_PARAMETER_LIST);
    std::unique_ptr<Node> optPList_parsed(parse_OptPList(optPList));
      s->append_kid(optPList_parsed.release());
    }
  
    expect_and_discard(TOK_RPAREN);
    expect_and_discard(TOK_LEFT_BRACKET);
    if(m_lexer->peek()->get_tag() != TOK_RIGHT_BRACKET) {
       Node *  statementList = new Node(AST_STATEMENT_LIST);
    std::unique_ptr<Node> sList_parse(parse_SList(statementList));
        s->append_kid(sList_parse.release());
    }
   
    expect_and_discard(TOK_RIGHT_BRACKET);
    return s;
 }


 Node *Parser2::parse_OptPList(Node * ast_) {
    //TODO OptPList →   PList                                     -- optional parameter list
    //TODO optPList →   ε
    Node *next_tok = m_lexer->peek();
    if(next_tok != nullptr) {
        parse_PList(ast_);
         return ast_;
    }
    return ast_;
 }

 Node *Parser2::parse_PList(Node * ast_) {
    //TODO PList →      ident                                     -- nonempty parameter list
    //TODO PList →      ident , PList
    std::unique_ptr<Node> ident(expect(static_cast<enum TokenKind>(TOK_IDENTIFIER)));
    ident->set_tag(AST_VARREF);
    ast_->append_kid(ident.release());
    Node *next_tok = m_lexer->peek();
    if(next_tok != nullptr && next_tok->get_tag() == TOK_COMMA) {   
      expect_and_discard(TOK_COMMA);
      return parse_PList(ast_);
    } 
    return ast_;
   
 }

 Node *Parser2::parse_SList(Node * ast_) {
    //TODO SList →      Stmt                                      -- statement list
    //TODO SList →      Stmt SList
    Node * parseStmt = parse_Stmt();
    ast_->append_kid(parseStmt);
    Node * next_tok  = m_lexer->peek();
    if(next_tok != nullptr && next_tok->get_tag() != TOK_RIGHT_BRACKET) {
      return parse_SList(ast_);
    }
    return ast_;
 }

 Node *Parser2::parse_OptArgList(Node * ast_) {
    //TODO OptArgList → ArgList                                   -- optional argument list
    //TODO OptArgList → ε
   Node *next_tok = m_lexer->peek();
    if(next_tok != nullptr) {
        parse_ArgList(ast_);
         return ast_;
    }
    return ast_;
 }


 Node *Parser2::parse_ArgList(Node * ast_) {
    //TODO ArgList →    L                                         -- nonempty argument list
    //TODO ArgList →    L , ArgList
    Node * parseL = parse_L();
    Node * next_tok = m_lexer->peek();
    ast_->append_kid(parseL);
    if(next_tok != nullptr && next_tok->get_tag() == TOK_COMMA) {
        expect_and_discard(TOK_COMMA);
        return parse_ArgList(ast_);
    }
    return parseL;
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

  //TODO F →          ident ( OptArgList )                      -- function call

  Node *next_tok = m_lexer->peek();
  if (next_tok == nullptr) {
    error_at_current_loc("Unexpected end of input looking for primary expression_F");
  }

  int tag = next_tok->get_tag();
  if (tag == TOK_INTEGER_LITERAL || tag == TOK_IDENTIFIER) {
    // F -> ^ number
    // F -> ^ ident
    
    std::unique_ptr<Node> tok(expect(static_cast<enum TokenKind>(tag)));
    if(tag == TOK_IDENTIFIER && m_lexer->peek()->get_tag() == TOK_LPAREN) {
        //F -> ident (OptArgList)
        std::unique_ptr<Node> fncall(new Node(AST_FNCALL));
        tok->set_tag(AST_VARREF);
        fncall->append_kid(tok.release());
        expect_and_discard(TOK_LPAREN);
        if(m_lexer->peek()->get_tag() != TOK_RPAREN){
           Node * parseArgList = new Node(AST_ARGLIST);
           std::unique_ptr<Node> optArgList(parse_OptArgList(parseArgList));
           fncall->append_kid(optArgList.release());
        }
        expect_and_discard(TOK_RPAREN);
        return fncall.release();
    }else {
      assert(tok->get_str().length() != 0);
      int ast_tag = tag == TOK_INTEGER_LITERAL ? AST_INT_LITERAL : AST_VARREF;
      std::unique_ptr<Node> ast(new Node(ast_tag));
      ast->set_str(tok->get_str());
      ast->set_loc(tok->get_loc());
      return ast.release();
    }
  } else if (tag == TOK_LPAREN) {
    expect_and_discard(TOK_LPAREN);
    std::unique_ptr<Node> ast(parse_A());
    expect_and_discard(TOK_RPAREN);
    return ast.release();
  } else if (tag == TOK_STRING) {
      std::unique_ptr<Node> string_literal(expect(static_cast<enum TokenKind>(TOK_STRING)));
      string_literal->set_tag(AST_STRING);
      return string_literal.release();

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
