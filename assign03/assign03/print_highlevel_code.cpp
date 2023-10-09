#include <cassert>
#include "highlevel_formatter.h"
#include "print_instruction_seq.h"
#include "print_highlevel_code.h"

PrintHighLevelCode::PrintHighLevelCode() {
}

PrintHighLevelCode::~PrintHighLevelCode() {
}

void PrintHighLevelCode::print_instructions(const std::shared_ptr<InstructionSequence> &iseq) {
  HighLevelFormatter formatter;
  PrintInstructionSequence print_iseq(&formatter);

  print_iseq.print(iseq.get());
}
