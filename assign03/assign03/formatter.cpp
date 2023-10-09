#include <stdexcept>
#include "cpputil.h"
#include "formatter.h"

Formatter::Formatter() {
}

Formatter::~Formatter() {
}

std::string Formatter::format_operand(const Operand &operand) const {
  switch (operand.get_kind()) {
  case Operand::IMM_IVAL:
    return cpputil::format("$%ld", operand.get_imm_ival());

  case Operand::LABEL:
    return operand.get_label();

  case Operand::IMM_LABEL:
    return cpputil::format("$%s", operand.get_label().c_str());

  default:
    {
      std::string exmsg = cpputil::format("Operand kind %d not handled", operand.get_kind());
      throw std::runtime_error(exmsg);
    }
  }
}
