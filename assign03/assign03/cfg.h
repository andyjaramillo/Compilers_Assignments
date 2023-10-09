#ifndef CFG_H
#define CFG_H

#include <cassert>
#include <vector>
#include <map>
#include <deque>
#include <string>
#include <memory>
#include "instruction.h"
#include "instruction_seq.h"
#include "operand.h"

enum BasicBlockKind {
  BASICBLOCK_ENTRY,     // special "entry" block
  BASICBLOCK_EXIT,      // special "exit" block
  BASICBLOCK_INTERIOR,  // normal basic block in the "interior" of the CFG
};

// A BasicBlock is an InstructionSequence in which only the last instruction
// can be a branch.
class BasicBlock : public InstructionSequence {
private:
  BasicBlockKind m_kind;
  unsigned m_id;
  std::string m_label;

  // indicates relative position of this basic block
  // in the original instruction sequence
  int m_code_order;

public:
  BasicBlock(BasicBlockKind kind, unsigned id, int code_order, const std::string &label = "");
  ~BasicBlock();

  BasicBlockKind get_kind() const;
  unsigned get_id() const;

  bool has_label() const;
  std::string get_label() const;

  // it is sometimes necessary to set a BasicBlock's label after it is created
  void set_label(const std::string &label);

  int get_code_order() const { return m_code_order; }
};

// Edges can be
//   - "fall through", meaning the target block's first instruction
//     follows the source block's last instruction in the original
//     InstructionSequence
//   - "branch", meaning that the target BasicBlock's first instruction
//     is *not* the immediate successor of the source block's last instruction
enum EdgeKind {
  EDGE_FALLTHROUGH,
  EDGE_BRANCH,
};

// An Edge is a predecessor/successor connection between a source BasicBlock
// and a target BasicBlock.
class Edge {
private:
  EdgeKind m_kind;
  BasicBlock *m_source, *m_target;

public:
  Edge(BasicBlock *source, BasicBlock *target, EdgeKind kind);
  ~Edge();

  EdgeKind get_kind() const { return m_kind; }
  BasicBlock *get_source() const { return m_source; }
  BasicBlock *get_target() const { return m_target; }
};

// ControlFlowGraph: graph of BasicBlocks connected by Edges.
// There are dedicated empty entry and exit blocks.
class ControlFlowGraph {
public:
  typedef std::vector<BasicBlock *> BlockList;
  typedef std::vector<Edge *> EdgeList;
  typedef std::map<const BasicBlock *, EdgeList> EdgeMap;

private:
  BlockList m_basic_blocks;
  BasicBlock *m_entry, *m_exit;
  EdgeMap m_incoming_edges;
  EdgeMap m_outgoing_edges;
  EdgeList m_empty_edge_list;

  // A "Chunk" is a collection of BasicBlocks
  // connected by fall-through edges.  All of the blocks
  // in a Chunk must be emitted contiguously in the
  // resulting InstructionSequence when the CFG is flattened.
  struct Chunk {
    ControlFlowGraph::BlockList blocks;
    bool is_exit;

    void append(BasicBlock *bb) {
      blocks.push_back(bb);
      if (bb->get_kind() == BASICBLOCK_EXIT) { is_exit = true; }
    }
    void prepend(BasicBlock *bb) {
      blocks.insert(blocks.begin(), bb);
      if (bb->get_kind() == BASICBLOCK_EXIT) { is_exit = true; }
    }

    Chunk *merge_with(Chunk *other) {
      Chunk *merged = new Chunk();
      for (auto i = blocks.begin(); i != blocks.end(); i++) {
        merged->append(*i);
      }
      for (auto i = other->blocks.begin(); i != other->blocks.end(); i++) {
        merged->append(*i);
      }
      return merged;
    }

    bool is_first(BasicBlock *bb) const {
      return !blocks.empty() && blocks.front() == bb;
    }

    bool is_last(BasicBlock *bb) const {
      return !blocks.empty() && blocks.back() == bb;
    }

    bool contains_exit_block() const { return is_exit; }
  };

public:
  ControlFlowGraph();
  ~ControlFlowGraph();

  // get total number of BasicBlocks (including entry and exit)
  unsigned get_num_blocks() const { return unsigned(m_basic_blocks.size()); }

  // Get pointer to the dedicated empty entry block
  BasicBlock *get_entry_block() const;

  // Get pointer to the dedicated empty exit block
  BasicBlock *get_exit_block() const;

  // Get block with specified id
  BasicBlock *get_block(unsigned id) const {
    assert(id < m_basic_blocks.size());
    return m_basic_blocks[id];
  }

  // iterator over pointers to BasicBlocks
  BlockList::const_iterator bb_begin() const { return m_basic_blocks.cbegin(); }
  BlockList::const_iterator bb_end() const   { return m_basic_blocks.cend(); }

  // Create a new BasicBlock: use BASICBLOCK_INTERIOR for all blocks
  // except for entry and exit. The label parameter should be a non-empty
  // string if the BasicBlock is reached via one or more branch instructions
  // (which should have this label as their Operand.)
  BasicBlock *create_basic_block(BasicBlockKind kind, int code_order, const std::string &label = "");

  // Create Edge of given kind from source to target
  Edge *create_edge(BasicBlock *source, BasicBlock *target, EdgeKind kind);

  // Look up edge from specified source block to target block:
  // returns a null pointer if no such block exists
  Edge *lookup_edge(BasicBlock *source, BasicBlock *target) const;

  // Get vector of all outgoing edges from given block
  const EdgeList &get_outgoing_edges(const BasicBlock *bb) const;

  // Get vector of all incoming edges to given block
  const EdgeList &get_incoming_edges(const BasicBlock *bb) const;

  // Return a "flat" InstructionSequence created from this ControlFlowGraph;
  // this is useful for optimization passes which create a transformed ControlFlowGraph
  std::shared_ptr<InstructionSequence> create_instruction_sequence() const;

private:
  std::vector<const BasicBlock *> get_blocks_in_code_order() const;
  bool can_use_original_block_order() const;
  std::shared_ptr<InstructionSequence> rebuild_instruction_sequence() const;
  std::shared_ptr<InstructionSequence> reconstruct_instruction_sequence() const;
  void append_basic_block(const std::shared_ptr<InstructionSequence> &iseq, const BasicBlock *bb, std::vector<bool> &finished_blocks) const;
  void append_chunk(const std::shared_ptr<InstructionSequence> &iseq, Chunk *chunk, std::vector<bool> &finished_blocks) const;
  void visit_successors(BasicBlock *bb, std::deque<BasicBlock *> &work_list) const;
  void delete_blocks();
  void delete_edges(EdgeMap &edge_map);
};

// ControlFlowGraphBuilder builds a ControlFlowGraph from an InstructionSequence.
class ControlFlowGraphBuilder {
private:
  std::shared_ptr<InstructionSequence> m_iseq;
  std::shared_ptr<ControlFlowGraph> m_cfg;
  // map of instruction index to pointer to BasicBlock starting at that instruction
  std::map<unsigned, BasicBlock *> m_basic_blocks;

  struct WorkItem {
    unsigned ins_index;
    BasicBlock *pred;
    EdgeKind edge_kind;
    std::string label;
  };

public:
  ControlFlowGraphBuilder(const std::shared_ptr<InstructionSequence> &iseq);
  virtual ~ControlFlowGraphBuilder();

  // Build a ControlFlowGraph from the original InstructionSequence, and return
  // a pointer to it
  std::shared_ptr<ControlFlowGraph> build();

  // Subclasses may override this method to specify which Instructions are
  // branches (i.e., have a control successor other than the next instruction
  // in the original InstructionSequence).  The default implementation assumes
  // that any Instruction with an operand of type Operand::LABEL
  // is a branch.
  virtual bool is_branch(Instruction *ins);

  // Subclasses must override this to check whether the given Instruction
  // is a function call.
  virtual bool is_function_call(Instruction *ins) = 0;

  // Subclasses must override this to check whether given Instruction
  // can fall through to the next instruction.  This method should return
  // true for all Instructions *except* unconditional branches.
  virtual bool falls_through(Instruction *ins) = 0;

private:
  BasicBlock *scan_basic_block(const WorkItem &item, const std::string &label);
  bool ends_in_branch(BasicBlock *bb);
  unsigned get_branch_target_index(BasicBlock *bb);
  bool falls_through(BasicBlock *bb);
};

// Build a high-level ControlFlowGraph from a high-level InstructionSequence.
class HighLevelControlFlowGraphBuilder : public ControlFlowGraphBuilder {
public:
  HighLevelControlFlowGraphBuilder(const std::shared_ptr<InstructionSequence> &iseq);
  virtual ~HighLevelControlFlowGraphBuilder();

  virtual bool is_function_call(Instruction *ins);
  virtual bool falls_through(Instruction *ins);
};

// Build a low-level ControlFlowGraph from a low-level InstructionSequence.
class LowLevelControlFlowGraphBuilder : public ControlFlowGraphBuilder {
public:
  LowLevelControlFlowGraphBuilder(const std::shared_ptr<InstructionSequence> &iseq);
  virtual ~LowLevelControlFlowGraphBuilder();

  virtual bool is_function_call(Instruction *ins);
  virtual bool falls_through(Instruction *ins);
};

// For debugging, print a textual representation of a ControlFlowGraph.
// Subclasses must override the print_basic_block method.
class ControlFlowGraphPrinter {
private:
  std::shared_ptr<ControlFlowGraph> m_cfg;

public:
  ControlFlowGraphPrinter(const std::shared_ptr<ControlFlowGraph> &cfg);
  ~ControlFlowGraphPrinter();

  void print();

  // These functions can be overridden to return an annotate
  // describing a fact about the entry and exit to
  // a particular basic block.
  virtual std::string get_block_begin_annotation(BasicBlock *bb);
  virtual std::string get_block_end_annotation(BasicBlock *bb);

  // The intended way to implement this method is to use a specialized
  // variant of PrintInstructionSequence.
  virtual void print_basic_block(BasicBlock *bb) = 0;
};

// Print a high-level ControlFlowGraph.
class HighLevelControlFlowGraphPrinter : public ControlFlowGraphPrinter {
public:
  HighLevelControlFlowGraphPrinter(const std::shared_ptr<ControlFlowGraph> &cfg);
  virtual ~HighLevelControlFlowGraphPrinter();

  virtual void print_basic_block(BasicBlock *bb);
};

// Print a low-level ControlFlowGraph.
class LowLevelControlFlowGraphPrinter : public ControlFlowGraphPrinter {
public:
  LowLevelControlFlowGraphPrinter(const std::shared_ptr<ControlFlowGraph> &cfg);
  virtual ~LowLevelControlFlowGraphPrinter();

  virtual void print_basic_block(BasicBlock *bb);
};

#endif // CFG_H
