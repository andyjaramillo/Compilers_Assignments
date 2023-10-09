#ifndef LIVE_MREGS_H
#define LIVE_MREGS_H

#include <string>
#include "instruction.h"
#include "lowlevel_defuse.h"
#include "lowlevel_formatter.h"
#include "dataflow.h"

class LiveMregsAnalysis : public BackwardAnalysis {
public:
  // There are only 16 mregs
  static const unsigned MAX_MREGS = 16;

  // Fact type is bitset of machine register numbers
  typedef std::bitset<MAX_MREGS> FactType;

  // The "top" fact is an unknown value that combines nondestructively
  // with known facts. For this analysis, it's the empty set.
  FactType get_top_fact() const { return FactType(); }

  // Combine live sets. For this analysis, we use union.
  FactType combine_facts(const FactType &left, const FactType &right) const {
    return left | right;
  }

  // Model an instruction.
  void model_instruction(Instruction *ins, FactType &fact) const {
    // Model an instruction (backwards).  If the instruction is a def,
    // the assigned-to mreg is killed.  Every mreg used in the instruction,
    // the mreg becomes alive (or is kept alive.)

    if (LowLevel::is_def(ins)) {
      std::vector<MachineReg> defs = LowLevel::get_def_mregs(ins);
      for (auto i = defs.begin(); i != defs.end(); ++i)
        fact.reset(unsigned(*i));
    }

    std::vector<MachineReg> uses = LowLevel::get_use_mregs(ins);
    for (auto i = uses.begin(); i != uses.end(); ++i)
      fact.set(unsigned(*i));
  }

  // Convert a dataflow fact to a string (for printing the CFG annotated with
  // dataflow facts)
  std::string fact_to_string(const FactType &fact) const {
    LowLevelFormatter ll_formatter;

    std::string s("{");
    for (unsigned i = 0; i < MAX_MREGS; i++) {
      if (fact.test(i)) {
        if (s != "{") { s += ","; }
        // Machine registers are shown using their 64-bit name
        Operand mreg_operand(Operand::MREG64, i);
        s += ll_formatter.format_operand(mreg_operand);
      }
    }
    s += "}";
    return s;
  }
};

typedef Dataflow<LiveMregsAnalysis> LiveMregs;

#endif // LIVE_MREGS_H
