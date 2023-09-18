#ifndef INTERP_H
#define INTERP_H

#include "ast.h"
#include "exceptions.h"
#include "value.h"
#include "environment.h"
class Node;
class Location;

class Interpreter {
private:
  Node *m_ast;

public:
  Interpreter(Node *ast_to_adopt);
  ~Interpreter();

  void analyze();
  Value execute();

private:
  // TODO: private member functions
  Environment block;
  int evaluate(Node *root);
  //this is the recursive function to allow pre order traversal in the AST Tree
  void analyzeBranch(Node * root);
  //A helper function that checks if the current two nodes are an integer or variable reference, and returns the result acordingly
  std::array<int, 2> int_literal_or_var_ref(Node* left, Node* right);
};

#endif // INTERP_H
