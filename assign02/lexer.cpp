#include "lexer.h"
#include "cpputil.h"
#include "exceptions.h"
#include "token.h"
#include <cassert>
#include <cctype>
#include <iostream>
#include <map>
#include <string>
////////////////////////////////////////////////////////////////////////
// Lexer implementation
////////////////////////////////////////////////////////////////////////

Lexer::Lexer(FILE *in, const std::string &filename)
    : m_in(in), m_filename(filename), m_line(1), m_col(1), m_eof(false) {}

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
  if (c == '\n') {
    m_line--;
    m_col = 99;
  } else {
    m_col--;
  }
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
    Node *tok =
        read_continued_token(TOK_IDENTIFIER, lexeme, line, col, isalnum);
    if (tok->get_str() == "var") {
      tok->set_tag(TOK_VAR);
    } else if(tok->get_str() == "function") {
      tok->set_tag(TOK_FUNCTION);
    } else if(tok->get_str()== "if"){
      tok->set_tag(TOK_IF);
    } else if(tok->get_str() == "else"){
      tok->set_tag(TOK_ELSE);
    } else if(tok->get_str() == "while"){
      tok->set_tag(TOK_WHILE);
    }

    // TODO: use set_tag to change the token kind if it's actually a keyword
    return tok;
  } else if (isdigit(c)) {
    return read_continued_token(TOK_INTEGER_LITERAL, lexeme, line, col,   isdigit);
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
    case ';':
      return token_create(TOK_SEMICOLON, lexeme, line, col);
    case '{':
      return token_create(TOK_LEFT_BRACKET, lexeme, line, col);
    case '}':
      return token_create(TOK_RIGHT_BRACKET, lexeme, line, col);
    case ',':
      return token_create(TOK_COMMA, lexeme, line, col);
    case '"':{
      std::string full_string = read_continued_string_character_token(TOK_STRING, lexeme, line, col);
      return token_create(TOK_STRING, full_string, line, col);
    }
    case '=': {
      std::string equals =
          read_continued_character_token(TOK_COMPARE, lexeme, line, col);
      return (equals == "=") ? token_create(TOK_EQUALS, equals, line, col)
                             : token_create(TOK_COMPARE, equals, line, col);
    }
    case '|': {
      std::string or_stub =
          read_continued_character_token(TOK_OR, lexeme, line, col);
      return token_create(TOK_OR, or_stub, line, col);
    }
    case '&': {
      std::string and_stub =
          read_continued_character_token(TOK_AND, lexeme, line, col);
      return token_create(TOK_AND, and_stub, line, col);
    }
    case '<': {
      std::string less_stub =
          read_continued_character_token(TOK_LESS, lexeme, line, col);
      return (less_stub == "<")
                 ? token_create(TOK_LESS, less_stub, line, col)
                 : token_create(TOK_LESSOREQUAL, less_stub, line, col);
    }
    case '>': {
      std::string greater_stub =
          read_continued_character_token(TOK_GREATER, lexeme, line, col);
      return (greater_stub == ">")
                 ? token_create(TOK_GREATER, greater_stub, line, col)
                 : token_create(TOK_GREATEROREQUAL, greater_stub, line, col);
    }
    case '!': {
      std::string not_stub =
          read_continued_character_token(TOK_NOTCOMPARE, lexeme, line, col);
      return token_create(TOK_NOTCOMPARE, not_stub, line, col);
    }
    // TODO: add cases for other kinds of tokens
    default:
      SyntaxError::raise(get_current_loc(), "Unrecognized character '%c'", c);
    }
  }
}

// Helper function to create a Node object to represent a token.
Node *Lexer::token_create(enum TokenKind kind, const std::string &lexeme,
                          int line, int col) {
  Node *token = new Node(kind, lexeme);
  Location source_info(m_filename, line, col);
  token->set_loc(source_info);
  return token;
}

// Read the continuation of a (possibly) multi-character token, such as
// an identifier or integer literal.  pred is a pointer to a predicate
// function to determine which characters are valid continuations.
Node *Lexer::read_continued_token(enum TokenKind kind,
                                  const std::string &lexeme_start, int line,
                                  int col, int (*pred)(int)) {
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

std::string Lexer::read_continued_character_token(
    enum TokenKind kind, const std::string &lexeme_start, int line, int col) {
  std::string lexeme(lexeme_start);
  int c = read();
  if (c >= 0 && (c == '=' || c == '&' || c == '|')) {
    lexeme.push_back(c);
  } else {
    unread(c);
  }
  return lexeme;
}

std::string Lexer::read_continued_string_character_token(
    enum TokenKind kind, const std::string &lexeme_start, int line, int col) {
  std::string lexeme(lexeme_start);
  int backslash = 92;

  for (;;) {
    int c = read();
    if (c >= 0 && c != '"') {
      if(c == backslash){
           //check if its one of the symbols
        int k = read();
        if(k == 'n' || k == 't' || k == '"' || k == 'r'){
          //special token
          lexeme.push_back(char(c));
          lexeme.push_back(char(k));
          continue;
        }else{
          //contains \other_symbol so we see backslash and other symbol as seperate
          unread(k);
          lexeme.push_back(char(c));
          continue;
        }
      } else {
         lexeme.push_back(char(c));
      }
    } else {  
       // token has finished
       lexeme.push_back(char(c));
       return lexeme;
    }
  }
}

// TODO: implement additional member functions if necessary
