#ifndef PRINT_HIGHLEVEL_CODE_H
#define PRINT_HIGHLEVEL_CODE_H

#include "print_code.h"

// Implementation of ModuleCollector that just prints high level
// code to stdout for informational/debugging purposes.
class PrintHighLevelCode : public PrintCode {
public:
  PrintHighLevelCode();
  virtual ~PrintHighLevelCode();

  virtual void print_instructions(const std::shared_ptr<InstructionSequence> &iseq);
};

#endif // PRINT_HIGHLEVEL_CODE_H
