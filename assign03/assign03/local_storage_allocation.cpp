#include <cassert>
#include "node.h"
#include "symtab.h"
#include "local_storage_allocation.h"

LocalStorageAllocation::LocalStorageAllocation()
  : m_total_local_storage(0U)
  , m_next_vreg(VREG_FIRST_LOCAL) {
}

LocalStorageAllocation::~LocalStorageAllocation() {
}

void LocalStorageAllocation::visit_declarator_list(Node *n) {
  // TODO: implement
}

void LocalStorageAllocation::visit_function_definition(Node *n) {
  // TODO: implement
}

void LocalStorageAllocation::visit_function_declaration(Node *n) {
  // don't allocate storage for parameters in a function declaration
}

void LocalStorageAllocation::visit_function_parameter(Node *n) {
  // TODO: implement
}

void LocalStorageAllocation::visit_statement_list(Node *n) {
  // TODO: implement
}

void LocalStorageAllocation::visit_struct_type_definition(Node *n) {
  // TODO: implement (if you are going to use this visitor to assign offsets for struct fields)
}

// TODO: implement private member functions
