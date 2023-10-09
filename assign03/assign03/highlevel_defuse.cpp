#include <cassert>
#include <set>
#include "instruction.h"
#include "operand.h"
#include "highlevel.h"
#include "highlevel_defuse.h"

namespace {

// High-level opcodes that do NOT have a destination operand
const std::set<HighLevelOpcode> NO_DEST = {
  HINS_nop,
  HINS_ret,
  HINS_jmp,
  HINS_call,
  HINS_enter,
  HINS_leave,
  HINS_cjmp_t,
  HINS_cjmp_f,
};

// Does the instruction have a destination operand?
bool has_dest_operand(HighLevelOpcode hl_opcode) {
  return NO_DEST.count(hl_opcode) == 0;
}

}

namespace HighLevel {

// A high-level instruction is a def if it has a destination operand,
// and the destination operand is a vreg.
bool is_def(Instruction *ins) {
  if (!has_dest_operand(HighLevelOpcode(ins->get_opcode())))
    return false;

  assert(ins->get_num_operands() > 0);
  Operand dest = ins->get_operand(0);

  return dest.get_kind() == Operand::VREG;
}

bool is_use(Instruction *ins, unsigned operand_index) {
  Operand operand = ins->get_operand(operand_index);

  if (operand_index == 0 && has_dest_operand(HighLevelOpcode(ins->get_opcode()))) {
    // special case: if the instruction has a destination operand, but the operand
    // is a memory reference, then the base register and index register (if either
    // are present) are refs
    return operand.is_memref() && (operand.has_base_reg() || operand.has_index_reg());
  }

  // in general, an operand is a ref if it mentions a virtual register
  return operand.has_base_reg() || operand.has_index_reg();
}

}
