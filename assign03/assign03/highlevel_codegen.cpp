#include <cassert>
#include "node.h"
#include "instruction.h"
#include "highlevel.h"
#include "ast.h"
#include "parse.tab.h"
#include "grammar_symbols.h"
#include "exceptions.h"
#include "local_storage_allocation.h"
#include "highlevel_codegen.h"


// Adjust an opcode for a basic type
HighLevelOpcode get_opcode(HighLevelOpcode base_opcode, const std::shared_ptr<Type> &type) {
  if (type->is_basic())
    return static_cast<HighLevelOpcode>(int(base_opcode) + int(type->get_basic_type_kind()));
  else if (type->is_pointer())
    return static_cast<HighLevelOpcode>(int(base_opcode) + int(BasicTypeKind::LONG));
  else
    RuntimeError::raise("attempt to use type '%s' as data in opcode selection", type->as_str().c_str());
}

HighLevelCodegen::HighLevelCodegen(int next_label_num, bool optimize)
  : m_next_label_num(next_label_num)
  , m_optimize(optimize)
  , m_hl_iseq(new InstructionSequence()) {
}

HighLevelCodegen::~HighLevelCodegen() {
}

void HighLevelCodegen::visit_function_definition(Node *n) {
  // generate the name of the label that return instructions should target
  std::string fn_name = n->get_kid(1)->get_str();
  m_return_label_name = ".L" + fn_name + "_return";

  unsigned total_local_storage = 0U;
/*
  total_local_storage = n->get_total_local_storage();
*/

  m_hl_iseq->append(new Instruction(HINS_enter, Operand(Operand::IMM_IVAL, total_local_storage)));

  // visit body
  visit(n->get_kid(3));

  m_hl_iseq->define_label(m_return_label_name);
  m_hl_iseq->append(new Instruction(HINS_leave, Operand(Operand::IMM_IVAL, total_local_storage)));
  m_hl_iseq->append(new Instruction(HINS_ret));

  // Perform high-level optimizations?
  if (m_optimize) {
  }

}

void HighLevelCodegen::visit_statement_list(Node *n) {
  // TODO: implement
}

void HighLevelCodegen::visit_expression_statement(Node *n) {
  // TODO: implement
}

void HighLevelCodegen::visit_return_statement(Node *n) {
  // TODO: implement
}

void HighLevelCodegen::visit_return_expression_statement(Node *n) {
  // A possible implementation:
/*
  Node *expr = n->get_kid(0);

  // generate code to evaluate the expression
  visit(expr);

  // move the computed value to the return value vreg
  HighLevelOpcode mov_opcode = get_opcode(HINS_mov_b, expr->get_type());
  m_hl_iseq->append(new Instruction(mov_opcode, Operand(Operand::VREG, LocalStorageAllocation::VREG_RETVAL), expr->get_operand()));

  // jump to the return label
  visit_return_statement(n);
*/
}

void HighLevelCodegen::visit_while_statement(Node *n) {
  // TODO: implement
}

void HighLevelCodegen::visit_do_while_statement(Node *n) {
  // TODO: implement
}

void HighLevelCodegen::visit_for_statement(Node *n) {
  // TODO: implement
}

void HighLevelCodegen::visit_if_statement(Node *n) {
  // TODO: implement
}

void HighLevelCodegen::visit_if_else_statement(Node *n) {
  // TODO: implement
}

void HighLevelCodegen::visit_binary_expression(Node *n) {
  // TODO: implement
}

void HighLevelCodegen::visit_unary_expression(Node *n) {
  // TODO: implement
}

void HighLevelCodegen::visit_function_call_expression(Node *n) {
  // TODO: implement
}

void HighLevelCodegen::visit_field_ref_expression(Node *n) {
  // TODO: implement
}

void HighLevelCodegen::visit_indirect_field_ref_expression(Node *n) {
  // TODO: implement
}

void HighLevelCodegen::visit_array_element_ref_expression(Node *n) {
  // TODO: implement
}

void HighLevelCodegen::visit_variable_ref(Node *n) {
  // TODO: implement
}

void HighLevelCodegen::visit_literal_value(Node *n) {
  // A partial implementation (note that this won't work correctly
  // for string constants!):
  /*
  LiteralValue val = n->get_literal_value();
  int vreg = next_temp_vreg();
  Operand dest(Operand::VREG, vreg);
  HighLevelOpcode mov_opcode = get_opcode(HINS_mov_b, n->get_type());
  m_hl_iseq->append(new Instruction(mov_opcode, dest, Operand(Operand::IMM_IVAL, val.get_int_value())));
  n->set_operand(dest);
  */
}

void HighLevelCodegen::visit_implicit_conversion(Node *n) {
  // TODO: implement
}

std::string HighLevelCodegen::next_label() {
  std::string label = ".L" + std::to_string(m_next_label_num++);
  return label;
}

// TODO: additional private member functions
