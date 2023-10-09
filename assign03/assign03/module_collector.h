#ifndef MODULE_COLLECTOR_H
#define MODULE_COLLECTOR_H

#include <memory>
#include "instruction_seq.h"
#include "type.h"

// A ModuleCollector object collects the global variables and functions
// resulting from a codegen pass. This base class can be overridden with
// different implementations depending on what we want to do with
// the code generation results.

class ModuleCollector {
public:
  ModuleCollector();
  virtual ~ModuleCollector();

  virtual void collect_string_constant(const std::string &name, const std::string &strval) = 0;
  virtual void collect_global_var(const std::string &name, const std::shared_ptr<Type> &type) = 0;
  virtual void collect_function(const std::string &name, const std::shared_ptr<InstructionSequence> &iseq) = 0;
};

#endif // MODULE_COLLECTOR_H
