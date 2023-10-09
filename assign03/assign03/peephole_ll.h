#ifndef PEEPHOLE_LL_H
#define PEEPHOLE_LL_H

#include <deque>
#include <vector>
#include "live_mregs.h"
#include "cfg_transform.h"

class PeepholeLowLevel : public ControlFlowGraphTransform {
private:
  // liveness info about machine registers
  LiveMregs m_live_mregs;

  // window of instructions from the original InstructionSequence
  std::deque<Instruction *> m_window;

  // number of patterns matched/transformations applied
  int m_num_matched;

public:
  PeepholeLowLevel(const std::shared_ptr<ControlFlowGraph> &cfg);
  virtual ~PeepholeLowLevel();

  virtual std::shared_ptr<InstructionSequence> transform_basic_block(const InstructionSequence *orig_bb);

  int get_num_matched() const { return m_num_matched; }

private:
  void emit_earliest_in_window(const std::shared_ptr<InstructionSequence> &result_iseq);
};

#endif // PEEPHOLE_LL_H
