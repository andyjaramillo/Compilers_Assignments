#ifndef PRINT_INSTRUCTION_SEQ_H
#define PRINT_INSTRUCTION_SEQ_H

class Formatter;
class InstructionSequence;

class PrintInstructionSequence {
private:
  const Formatter *m_formatter;

public:
  PrintInstructionSequence(const Formatter *formatter);
  ~PrintInstructionSequence();

  void print(const InstructionSequence *iseq);

  virtual std::string get_instruction_annotation(const InstructionSequence *iseq, Instruction *ins);
};

#endif // PRINT_INSTRUCTION_SEQ_H
