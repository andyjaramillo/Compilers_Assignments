#ifndef PRINT_LOWLEVEL_CODE_H
#define PRINT_LOWLEVEL_CODE_H

#include "print_code.h"

// Implementation of ModuleCollector that prints low-level
// (x86-64) code
class PrintLowLevelCode : public PrintCode {
public:
  PrintLowLevelCode();
  virtual ~PrintLowLevelCode();

  virtual void print_instructions(const std::shared_ptr<InstructionSequence> &iseq);
};

#endif // PRINT_LOWLEVEL_CODE_H
