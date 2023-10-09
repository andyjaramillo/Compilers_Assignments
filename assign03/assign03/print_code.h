#ifndef PRINT_CODE_H
#define PRINT_CODE_H

#include "module_collector.h"

// Abstract base class for ModuleCollector which print generated code
class PrintCode : public ModuleCollector {
private:
  enum Mode {
    NONE, RODATA, DATA, CODE,
  };
  Mode m_mode;

public:
  PrintCode();
  virtual ~PrintCode();

  virtual void collect_string_constant(const std::string &name, const std::string &strval);
  virtual void collect_global_var(const std::string &name, const std::shared_ptr<Type> &type);
  virtual void collect_function(const std::string &name, const std::shared_ptr<InstructionSequence> &iseq);

  // This can be overridden depending on whether we're printing high-level
  // or low-level instructions
  virtual void print_instructions(const std::shared_ptr<InstructionSequence> &iseq) = 0;

private:
  void set_mode(Mode mode);
};

#endif // PRINT_CODE_H
