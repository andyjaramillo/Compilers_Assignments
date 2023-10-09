#ifndef DATAFLOW_H
#define DATAFLOW_H

#include <cassert>
#include <algorithm>
#include <memory>
#include <vector>
#include <bitset>
#include "cfg.h"

// Dataflow analysis direction
enum class DataflowDirection {
  FORWARD,
  BACKWARD,
};

// Forward navigation in the control-flow graph (from predecessors
// towards successors.)
class ForwardNavigation {
public:
  const BasicBlock *get_start_block(const std::shared_ptr<ControlFlowGraph> &cfg) const {
    return cfg->get_entry_block();
  }

  const ControlFlowGraph::EdgeList &get_edges(const std::shared_ptr<ControlFlowGraph> &cfg, const BasicBlock *bb) const {
    return cfg->get_outgoing_edges(bb);
  }

  const BasicBlock *get_block(const Edge *edge) const {
    return edge->get_target();
  }
};

// Backward navigation in the control-flow graph (from successors
// back to predecessors.)
class BackwardNavigation {
public:
  const BasicBlock *get_start_block(const std::shared_ptr<ControlFlowGraph> &cfg) const {
    return cfg->get_exit_block();
  }

  const ControlFlowGraph::EdgeList &get_edges(const std::shared_ptr<ControlFlowGraph> &cfg, const BasicBlock *bb) const {
    return cfg->get_incoming_edges(bb);
  }

  const BasicBlock *get_block(const Edge *edge) const {
    return edge->get_source();
  }
};

// Base class for forward analyses.
class ForwardAnalysis {
public:
  // Analysis direction is forward
  const static DataflowDirection DIRECTION = DataflowDirection::FORWARD;

  // iterator type for iterating over instructions in a basic block
  typedef InstructionSequence::const_iterator InstructionIterator;

  // Get begin/end iterators according to desired iteration order
  // (in this case, forward)
  InstructionIterator begin(const BasicBlock *bb) const { return bb->cbegin(); }
  InstructionIterator end(const BasicBlock *bb) const { return bb->cend(); }

  // Logical navigation forward and backward (program order)
  ForwardNavigation LOGICAL_FORWARD;
  BackwardNavigation LOGICAL_BACKWARD;
};

// Base class for backward analyses.
class BackwardAnalysis {
public:
  // Analysis direction is backward
  const static DataflowDirection DIRECTION = DataflowDirection::BACKWARD;

  // iterator type for iterating over instructions in a basic block
  typedef InstructionSequence::const_reverse_iterator InstructionIterator;

  // Get begin/end iterators according to desired iteration order
  // (in this case, backward)
  InstructionIterator begin(const BasicBlock *bb) const { return bb->crbegin(); }
  InstructionIterator end(const BasicBlock *bb) const { return bb->crend(); }

  // Logical navigation forward and backward (reverse of program order)
  BackwardNavigation LOGICAL_FORWARD;
  ForwardNavigation LOGICAL_BACKWARD;
};

// An instance of Dataflow performs a dataflow analysis on the basic blocks
// of a control flow graph and provides an interface for querying
// dataflow facts at arbitrary points.  The Analysis object (instance of the
// Analysis type parameter) provides the concrete details about how the
// analysis is performed:
//
//   - the fact type
//   - how dataflow facts are combined
//   - how instructions are modeled
//
// etc.
template<typename Analysis>
class Dataflow {
public:
  // Data type representing a dataflow fact
  typedef typename Analysis::FactType FactType;

  // We assume there won't be more than this many basic blocks.
  static const unsigned MAX_BLOCKS = 1024;

private:
  // The Analysis object encapsulates all of the details about the
  // analysis to be performed: direction (forward or backward),
  // the fact type, how to model an instruction, how to combine
  // dataflow facts, etc.
  Analysis m_analysis;

  // The control flow graph to analyze
  std::shared_ptr<ControlFlowGraph> m_cfg;

  // facts at end and beginning of each basic block
  std::vector<FactType> m_endfacts, m_beginfacts;

  // block iteration order
  std::vector<unsigned> m_iter_order;

public:
  Dataflow(const std::shared_ptr<ControlFlowGraph> &cfg);
  ~Dataflow();

  // execute the analysis
  void execute();

  // get dataflow fact at end of specified block
  const FactType &get_fact_at_end_of_block(const BasicBlock *bb) const;

  // get dataflow fact at beginning of specific block
  const FactType &get_fact_at_beginning_of_block(const BasicBlock *bb) const;

  // get dataflow fact after specified instruction
  FactType get_fact_after_instruction(const BasicBlock *bb, Instruction *ins) const;

  // get dataflow fact before specified instruction
  FactType get_fact_before_instruction(const BasicBlock *bb, Instruction *ins) const;

  // convert dataflow fact to a string
  static std::string fact_to_string(const FactType &fact);

private:
  // Helpers to get the vector containing the facts known at "logical"
  // beginning and end of each basic block.  In this context, "beginning" means
  // where analysis of the block should start (beginning for forward analyses,
  // end for backward analyses), "end" means where analysis ends.

  std::vector<typename Analysis::FactType> &get_logical_begin_facts() {
    return Analysis::DIRECTION == DataflowDirection::FORWARD
      ? m_beginfacts
      : m_endfacts;
  }

  std::vector<typename Analysis::FactType> &get_logical_end_facts() {
    return Analysis::DIRECTION == DataflowDirection::FORWARD
      ? m_endfacts
      : m_beginfacts;
  }

  const std::vector<typename Analysis::FactType> &get_logical_begin_facts() const {
    return Analysis::DIRECTION == DataflowDirection::FORWARD
      ? m_beginfacts
      : m_endfacts;
  }

  const std::vector<typename Analysis::FactType> &get_logical_end_facts() const {
    return Analysis::DIRECTION == DataflowDirection::FORWARD
      ? m_endfacts
      : m_beginfacts;
  }

  // Helper for get_fact_after_instruction() and get_fact_before_instruction()
  FactType get_instruction_fact(const BasicBlock *bb, Instruction *ins, bool after_in_logical_order) const;

  // Compute the iteration order
  void compute_iter_order();

  // Postorder traversal on the CFG (or reversed CFG, depending on
  // analysis direction)
  void postorder_on_cfg(std::bitset<MAX_BLOCKS> &visited, const BasicBlock *bb);
};

template<typename Analysis>
Dataflow<Analysis>::Dataflow(const std::shared_ptr<ControlFlowGraph> &cfg)
  : m_cfg(cfg) {
  for (unsigned i = 0; i < cfg->get_num_blocks(); ++i) {
    m_beginfacts.push_back(m_analysis.get_top_fact());
    m_endfacts.push_back(m_analysis.get_top_fact());
  }
}

template<typename Analysis>
Dataflow<Analysis>::~Dataflow() {
}

template<typename Analysis>
void Dataflow<Analysis>::execute() {
  compute_iter_order();

  std::vector<FactType> &logical_begin_facts = get_logical_begin_facts(),
                        &logical_end_facts = get_logical_end_facts();

  const auto &to_logical_predecessors = m_analysis.LOGICAL_BACKWARD;

  bool done = false;

  unsigned num_iters = 0;
  while (!done) {
    num_iters++;
    bool change = false;

    // For each block (according to the iteration order)...
    for (auto i = m_iter_order.begin(); i != m_iter_order.end(); i++) {
      unsigned id = *i;
      const BasicBlock *bb = m_cfg->get_block(id);

      // Combine facts known from control edges from the "logical" predecessors
      // (which are the successors for backward analyses)
      FactType fact = m_analysis.get_top_fact();
      const ControlFlowGraph::EdgeList &logical_predecessor_edges = to_logical_predecessors.get_edges(m_cfg, bb);
      for (auto j = logical_predecessor_edges.cbegin(); j != logical_predecessor_edges.cend(); j++) {
        const Edge *e = *j;
        const BasicBlock *logical_predecessor = to_logical_predecessors.get_block(e);
        fact = m_analysis.combine_facts(fact, logical_end_facts[logical_predecessor->get_id()]);
      }

      // Update (currently-known) fact at the "beginning" of this basic block
      // (which will actually be the end of the basic block for backward analyses)
      logical_begin_facts[id] = fact;

      // For each Instruction in the basic block (in the appropriate analysis order)...
      for (auto j = m_analysis.begin(bb); j != m_analysis.end(bb); ++j) {
        Instruction *ins = *j;

        // model the instruction
        m_analysis.model_instruction(ins, fact);
      }

      // Did the fact at the logical "end" of the block change?
      // If so, at least one more round will be needed.
      if (fact != logical_end_facts[id]) {
        change = true;
        logical_end_facts[id] = fact;
      }
    }

    // If there was no change in the computed dataflow fact at the beginning
    // of any block, then the analysis has terminated
    if (!change) {
      done = true;
    }
  }
}

template<typename Analysis>
const typename Dataflow<Analysis>::FactType &Dataflow<Analysis>::get_fact_at_end_of_block(const BasicBlock *bb) const {
  return m_endfacts.at(bb->get_id());
}

template<typename Analysis>
const typename Dataflow<Analysis>::FactType &Dataflow<Analysis>::get_fact_at_beginning_of_block(const BasicBlock *bb) const {
  return m_beginfacts.at(bb->get_id());
}

template<typename Analysis>
typename Dataflow<Analysis>::FactType Dataflow<Analysis>::get_fact_after_instruction(const BasicBlock *bb, Instruction *ins) const {
  // For a backward analysis, we want the fact that is true "before" (logically)
  // the specified instruction.
  bool after_in_logical_order = (Analysis::DIRECTION == DataflowDirection::FORWARD);
  return get_instruction_fact(bb, ins, after_in_logical_order);
}

template<typename Analysis>
typename Dataflow<Analysis>::FactType Dataflow<Analysis>::get_fact_before_instruction(const BasicBlock *bb, Instruction *ins) const {
  // For a backward analysis, we want the fact that is true "after" (logically)
  // the specified instruction.
  bool after_in_logical_order = (Analysis::DIRECTION == DataflowDirection::BACKWARD);
  return get_instruction_fact(bb, ins, after_in_logical_order);
}

template<typename Analysis>
std::string Dataflow<Analysis>::fact_to_string(const FactType &fact) {
  Analysis analysis;
  return analysis.fact_to_string(fact);
}

// Compute the dataflow fact immediately before or after the specified instruction,
// in the "logical" order (corresponding to the analysis direction) rather than
// program order.
template<typename Analysis>
typename Analysis::FactType Dataflow<Analysis>::get_instruction_fact(const BasicBlock *bb,
                                                                     Instruction *ins,
                                                                     bool after_in_logical_order) const {
  const std::vector<FactType> &logical_begin_facts = get_logical_begin_facts();

  FactType fact = logical_begin_facts[bb->get_id()];

  for (auto i = m_analysis.begin(bb); i != m_analysis.end(bb); ++i) {
    Instruction *bb_ins = *i;
    bool at_instruction = (bb_ins == ins);

    if (at_instruction && !after_in_logical_order) break;

    m_analysis.model_instruction(bb_ins, fact);

    if (at_instruction) {
      assert(after_in_logical_order);
      break;
    }
  }

  return fact;
}

template<typename Analysis>
void Dataflow<Analysis>::compute_iter_order() {
  std::bitset<MAX_BLOCKS> visited;

  const auto &to_logical_successors = m_analysis.LOGICAL_FORWARD;

  const BasicBlock *logical_entry_block = to_logical_successors.get_start_block(m_cfg);

  postorder_on_cfg(visited, logical_entry_block);

  std::reverse(m_iter_order.begin(), m_iter_order.end());
}

template<typename Analysis>
void Dataflow<Analysis>::postorder_on_cfg(std::bitset<MAX_BLOCKS> &visited, const BasicBlock *bb) {
  const auto &to_logical_successors = m_analysis.LOGICAL_FORWARD;

  // already arrived at this block?
  if (visited.test(bb->get_id())) {
    return;
  }

  // this block is now guaranteed to be visited
  visited.set(bb->get_id());

  // recursively visit (logical) successors
  const ControlFlowGraph::EdgeList &logical_successor_edges = to_logical_successors.get_edges(m_cfg, bb);
  for (auto i = logical_successor_edges.begin(); i != logical_successor_edges.end(); i++) {
    const Edge *e = *i;
    const BasicBlock *next_bb = to_logical_successors.get_block(e);
    postorder_on_cfg(visited, next_bb);
  }

  // add this block to the order
  m_iter_order.push_back(bb->get_id());
}

#endif // DATAFLOW_H
