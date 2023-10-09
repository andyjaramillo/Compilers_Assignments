#ifndef LOWLEVEL_FORMATTER_H
#define LOWLEVEL_FORMATTER_H

#include "formatter.h"

class LowLevelFormatter : public Formatter {
public:
  LowLevelFormatter();
  virtual ~LowLevelFormatter();

  virtual std::string format_operand(const Operand &operand) const;
  virtual std::string format_instruction(const Instruction *ins) const;
};

#endif // LOWLEVEL_FORMATTER_H
