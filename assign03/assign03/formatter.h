#ifndef FORMATTER_H
#define FORMATTER_H

#include "operand.h"
class Instruction;

// A Formatter turns Operand and Instruction objects into
// strings, which in turn allows high-level and low-level code
// to be printed.
class Formatter {
public:
  Formatter();
  virtual ~Formatter();

  // The default implementation of format_operand could be useful
  // for immediate values and labels.
  virtual std::string format_operand(const Operand &operand) const;

  virtual std::string format_instruction(const Instruction *ins) const = 0;
};

#endif // FORMATTER_H
