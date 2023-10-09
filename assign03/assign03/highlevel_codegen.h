#include <string>
#include <memory>
#include "highlevel.h"
#include "instruction_seq.h"
#include "ast_visitor.h"

// A HighLevelCodegen visitor generates high-level IR code for
// a single function. Code generation is initiated by visiting
// a function definition AST node.
class HighLevelCodegen : public ASTVisitor {
private:
  int m_next_label_num;
  bool m_optimize;
  std::string m_return_label_name; // name of the label that return instructions should target
  std::shared_ptr<InstructionSequence> m_hl_iseq;

public:
  // the next_label_num controls where the next_label() member function
  HighLevelCodegen(int next_label_num, bool optimize);
  virtual ~HighLevelCodegen();

  std::shared_ptr<InstructionSequence> get_hl_iseq() { return m_hl_iseq; }

  int get_next_label_num() const { return m_next_label_num; }

  virtual void visit_function_definition(Node *n);
  virtual void visit_statement_list(Node *n);
  virtual void visit_expression_statement(Node *n);
  virtual void visit_return_statement(Node *n);
  virtual void visit_return_expression_statement(Node *n);
  virtual void visit_while_statement(Node *n);
  virtual void visit_do_while_statement(Node *n);
  virtual void visit_for_statement(Node *n);
  virtual void visit_if_statement(Node *n);
  virtual void visit_if_else_statement(Node *n);
  virtual void visit_binary_expression(Node *n);
  virtual void visit_unary_expression(Node *n);
  virtual void visit_function_call_expression(Node *n);
  virtual void visit_field_ref_expression(Node *n);
  virtual void visit_indirect_field_ref_expression(Node *n);
  virtual void visit_array_element_ref_expression(Node *n);
  virtual void visit_variable_ref(Node *n);
  virtual void visit_literal_value(Node *n);
  virtual void visit_implicit_conversion(Node *n);

private:
  std::string next_label();
  // TODO: additional private member functions
};
