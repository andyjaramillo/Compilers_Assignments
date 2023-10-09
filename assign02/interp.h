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
  Environment * block;
  Value recursive_execute(Node* ast, Environment * current_block);
  Value evaluate(Node *root, Environment * current_block);
  //this is the recursive function to allow pre order traversal in the AST Tree
  void analyzeBranch(Node * root, Environment * current_block);
  bool validLoopCondition(Node * loopCondition, Environment * current_block);
  //A helper function that checks if the current two nodes are an integer or variable reference, and returns the result acordingly
  std::array<int, 2> int_literal_or_var_ref(Node* left, Node* right);

  Value childNode();

  static Value intrinsic_print(Value args[], unsigned num_args, const Location &loc, Interpreter *interp);
  static Value intrinsic_println(Value args[], unsigned num_args, const Location &loc, Interpreter *interp);
  static Value intrinsic_readint(Value args[], unsigned num_args, const Location &loc, Interpreter *interp);
  static Value intrinsic_mkarr(Value args[], unsigned num_args, const Location &loc, Interpreter *interp);
  static Value intrinsic_len(Value args[], unsigned num_args, const Location &loc, Interpreter *interp);
  static Value intrinsic_get(Value args[], unsigned num_args, const Location &loc, Interpreter *interp);
  static Value intrinsic_set(Value args[], unsigned num_args, const Location &loc, Interpreter *interp);
  static Value intrinsic_push(Value args[], unsigned num_args, const Location &loc, Interpreter *interp);
  static Value intrinsic_pop(Value args[], unsigned num_args, const Location &loc, Interpreter *interp);
  static Value intrinsic_substr(Value args[], unsigned num_args, const Location &loc, Interpreter *interp);
  static Value intrinsic_strcat(Value args[], unsigned num_args, const Location &loc, Interpreter *interp);
  static Value intrinsic_strlen(Value args[], unsigned num_args, const Location &loc, Interpreter *interp);
  bool checkIfInstrinsic(std::string varRef);
};

#endif // INTERP_H