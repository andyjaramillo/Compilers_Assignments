#ifndef LIVE_VREGS_H
#define LIVE_VREGS_H

#include <string>
#include "instruction.h"
#include "highlevel_defuse.h"
#include "dataflow.h"

class LiveVregsAnalysis : public BackwardAnalysis {
public:
  // We assume that there are never more than this many vregs used
  static const unsigned MAX_VREGS = 256;

  // Fact type is a bitset of live virtual register numbers
  typedef std::bitset<MAX_VREGS> FactType;

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
    // the assigned-to vreg is killed.  Every vreg used in the instruction,
    // the vreg becomes alive (or is kept alive.)

    if (HighLevel::is_def(ins)) {
      Operand operand = ins->get_operand(0);
      assert(operand.has_base_reg());
      fact.reset(operand.get_base_reg());
    }

    for (unsigned i = 0; i < ins->get_num_operands(); i++) {
      if (HighLevel::is_use(ins, i)) {
        Operand operand = ins->get_operand(i);

        assert(operand.has_base_reg());
        fact.set(operand.get_base_reg());

        if (operand.has_index_reg()) {
          fact.set(operand.get_index_reg());
        }
      }
    }
  }

  // Convert a dataflow fact to a string (for printing the CFG annotated with
  // dataflow facts)
  std::string fact_to_string(const FactType &fact) const {
    std::string s("{");
    for (unsigned i = 0; i < MAX_VREGS; i++) {
      if (fact.test(i)) {
        if (s != "{") { s += ","; }
        s += std::to_string(i);
      }
    }
    s += "}";
    return s;
  }
};

typedef Dataflow<LiveVregsAnalysis> LiveVregs;

#endif // LIVE_VREGS_H
