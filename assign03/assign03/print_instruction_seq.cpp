#include "formatter.h"
#include "instruction.h"
#include "instruction_seq.h"
#include "print_instruction_seq.h"

PrintInstructionSequence::PrintInstructionSequence(const Formatter *formatter)
  : m_formatter(formatter) {
}

PrintInstructionSequence::~PrintInstructionSequence() {
}

void PrintInstructionSequence::print(const InstructionSequence *iseq) {
  for (auto i = iseq->cbegin(); i != iseq->cend(); i++) {
    // print label if there is one
    if (i.has_label()) {
      printf("%s:\n", i.get_label().c_str());
    }

    // print formatted instruction
    std::string formatted_ins = m_formatter->format_instruction(*i);
    printf("\t%s", formatted_ins.c_str());

    // if there is an "annotation" for the instruction, print it
    std::string annotation = get_instruction_annotation(iseq, *i);
    if (!annotation.empty()) {
      size_t len = formatted_ins.size();
      if (len < 28) {
        printf("%s", "                            " + len);
      }
      printf(" /* %s */", annotation.c_str());
    }

    printf("\n");
  }
}

std::string PrintInstructionSequence::get_instruction_annotation(const InstructionSequence *iseq, Instruction *ins) {
  return "";
}
