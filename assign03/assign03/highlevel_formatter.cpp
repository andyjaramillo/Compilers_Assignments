#include <stdexcept>
#include "cpputil.h"
#include "instruction.h"
#include "highlevel.h"
#include "highlevel_formatter.h"

HighLevelFormatter::HighLevelFormatter() {
}

HighLevelFormatter::~HighLevelFormatter() {
}

std::string HighLevelFormatter::format_operand(const Operand &operand) const {
  std::string s;

  switch (operand.get_kind()) {
  case Operand::VREG:
    s = cpputil::format("vr%d", operand.get_base_reg());
    break;
  case Operand::VREG_MEM:
    s = cpputil::format("(vr%d)", operand.get_base_reg());
    break;
  case Operand::VREG_MEM_IDX:
    s = cpputil::format("(vr%d, vr%d)", operand.get_base_reg(), operand.get_index_reg());
    break;
  case Operand::VREG_MEM_OFF:
    s = cpputil::format("%ld(vr%d)", operand.get_imm_ival(), operand.get_base_reg());
    break;
  default:
    s = Formatter::format_operand(operand);
  }

  return s;
}

std::string HighLevelFormatter::format_instruction(const Instruction *ins) const {
  HighLevelOpcode opcode = HighLevelOpcode(ins->get_opcode());

  const char *mnemonic_ptr = highlevel_opcode_to_str(opcode);
  if (mnemonic_ptr == nullptr) {
    std::string exmsg = cpputil::format("Unknown highlevel opcode: %d", ins->get_opcode());
    throw std::runtime_error(exmsg);
  }
  std::string mnemonic(mnemonic_ptr);

  std::string buf;

  buf += mnemonic;
  // pad mnemonics to 8 characters
  unsigned padding = (mnemonic.size() < 8U) ? 8U - mnemonic.size() : 0;
  buf += ("         " + (8U - padding));
  for (unsigned i = 0; i < ins->get_num_operands(); i++) {
    if (i > 0) {
      buf += ", ";
    }
    buf += format_operand(ins->get_operand(i));
  }

  return buf;
}
