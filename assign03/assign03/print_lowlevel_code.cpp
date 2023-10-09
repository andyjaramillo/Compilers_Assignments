#include <cassert>
#include "lowlevel_formatter.h"
#include "print_instruction_seq.h"
#include "print_lowlevel_code.h"

PrintLowLevelCode::PrintLowLevelCode() {
}

PrintLowLevelCode::~PrintLowLevelCode() {
}

void PrintLowLevelCode::print_instructions(const std::shared_ptr<InstructionSequence> &iseq) {
  LowLevelFormatter formatter;
  PrintInstructionSequence print_iseq(&formatter);

  print_iseq.print(iseq.get());
}
