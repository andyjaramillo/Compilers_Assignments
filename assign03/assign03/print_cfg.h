#ifndef PRINT_CFG_H
#define PRINT_CFG_H

#include "cfg.h"
#include "print_code.h"

// ModuleCollector implementation which prints CFGs of the
// high-level code.
class PrintHighLevelCFG : public PrintCode {
public:
  PrintHighLevelCFG();
  virtual ~PrintHighLevelCFG();

  virtual void print_instructions(const std::shared_ptr<InstructionSequence> &iseq);

  virtual void print_cfg(const std::shared_ptr<ControlFlowGraph> &cfg);
};

// ModuleCollector implementation which prints CFGs of the
// high-level code, annotated with liveness dataflow results.
class PrintHighLevelCFGWithLiveness : public PrintHighLevelCFG {
public:
  PrintHighLevelCFGWithLiveness();
  virtual ~PrintHighLevelCFGWithLiveness();

  virtual void print_cfg(const std::shared_ptr<ControlFlowGraph> &cfg);
};

// ModuleCollector implementation which prints CFGs of the
// low-level code.
class PrintLowLevelCFG : public PrintCode {
public:
  PrintLowLevelCFG();
  virtual ~PrintLowLevelCFG();

  virtual void print_instructions(const std::shared_ptr<InstructionSequence> &iseq);

  virtual void print_cfg(const std::shared_ptr<ControlFlowGraph> &cfg);
};

// ModuleCollector implementation which prints CFGs of the
// low-level code, annotated with liveness dataflow results.
class PrintLowLevelCFGWithLiveness : public PrintLowLevelCFG {
public:
  PrintLowLevelCFGWithLiveness();
  virtual ~PrintLowLevelCFGWithLiveness();

  virtual void print_cfg(const std::shared_ptr<ControlFlowGraph> &cfg);
};

#endif // PRINT_CFG_H
