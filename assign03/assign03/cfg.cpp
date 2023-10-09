#include <cassert>
#include <cstdio>
#include <algorithm>
#include <iterator>
#include "cpputil.h"
#include "exceptions.h"
#include "highlevel.h"
#include "lowlevel.h"
#include "print_instruction_seq.h"
#include "highlevel_formatter.h"
#include "lowlevel_formatter.h"
#include "cfg.h"

////////////////////////////////////////////////////////////////////////
// BasicBlock implementation
////////////////////////////////////////////////////////////////////////

BasicBlock::BasicBlock(BasicBlockKind kind, unsigned id, int code_order, const std::string &label)
  : m_kind(kind)
  , m_id(id)
  , m_label(label)
  , m_code_order(code_order) {
}

BasicBlock::~BasicBlock() {
}

BasicBlockKind BasicBlock::get_kind() const {
  return m_kind;
}

unsigned BasicBlock::get_id() const {
  return m_id;
}

bool BasicBlock::has_label() const {
  return !m_label.empty();
}

std::string BasicBlock::get_label() const {
  return m_label;
}

void BasicBlock::set_label(const std::string &label) {
  assert(!has_label());
  m_label = label;
}

////////////////////////////////////////////////////////////////////////
// Edge implementation
////////////////////////////////////////////////////////////////////////

Edge::Edge(BasicBlock *source, BasicBlock *target, EdgeKind kind)
  : m_kind(kind)
  , m_source(source)
  , m_target(target) {
}

Edge::~Edge() {
}

////////////////////////////////////////////////////////////////////////
// ControlFlowGraph implementation
////////////////////////////////////////////////////////////////////////

ControlFlowGraph::ControlFlowGraph()
  : m_entry(nullptr)
  , m_exit(nullptr) {
}

ControlFlowGraph::~ControlFlowGraph() {
  // The ControlFlowGraph owns its BasicBlocks and Edges.
  // Note that because edges are added to both m_incoming_edges and
  // m_outgoing_edges, it is sufficient to just iterate through
  // one of them to find all edges to delete.
  delete_blocks();
  delete_edges(m_incoming_edges);
}

BasicBlock *ControlFlowGraph::get_entry_block() const {
  return m_entry;
}

BasicBlock *ControlFlowGraph::get_exit_block() const {
  return m_exit;
}

BasicBlock *ControlFlowGraph::create_basic_block(BasicBlockKind kind, int code_order, const std::string &label) {
  BasicBlock *bb = new BasicBlock(kind, unsigned(m_basic_blocks.size()), code_order, label);
  m_basic_blocks.push_back(bb);
  if (bb->get_kind() == BASICBLOCK_ENTRY) {
    assert(m_entry == nullptr);
    m_entry = bb;
  }
  if (bb->get_kind() == BASICBLOCK_EXIT) {
    assert(m_exit == nullptr);
    m_exit = bb;
  }
  return bb;
}

Edge *ControlFlowGraph::create_edge(BasicBlock *source, BasicBlock *target, EdgeKind kind) {
  // make sure BasicBlocks belong to this ControlFlowGraph
  assert(std::find(m_basic_blocks.begin(), m_basic_blocks.end(), source) != m_basic_blocks.end());
  assert(std::find(m_basic_blocks.begin(), m_basic_blocks.end(), target) != m_basic_blocks.end());

  // make sure this Edge doesn't already exist
  assert(lookup_edge(source, target) == nullptr);

  // create the edge, add it to outgoing/incoming edge maps
  Edge *e = new Edge(source, target, kind);
  m_outgoing_edges[source].push_back(e);
  m_incoming_edges[target].push_back(e);

  return e;
}

Edge *ControlFlowGraph::lookup_edge(BasicBlock *source, BasicBlock *target) const {
  auto i = m_outgoing_edges.find(source);
  if (i == m_outgoing_edges.cend()) {
    return nullptr;
  }
  const EdgeList &outgoing = i->second;
  for (auto j = outgoing.cbegin(); j != outgoing.cend(); j++) {
    Edge *e = *j;
    assert(e->get_source() == source);
    if (e->get_target() == target) {
      return e;
    }
  }
  return nullptr;
}

const ControlFlowGraph::EdgeList &ControlFlowGraph::get_outgoing_edges(const BasicBlock *bb) const {
  EdgeMap::const_iterator i = m_outgoing_edges.find(bb);
  return i == m_outgoing_edges.end() ? m_empty_edge_list : i->second;
}

const ControlFlowGraph::EdgeList &ControlFlowGraph::get_incoming_edges(const BasicBlock *bb) const {
  EdgeMap::const_iterator i = m_incoming_edges.find(bb);
  return i == m_incoming_edges.end() ? m_empty_edge_list : i->second;
}

std::shared_ptr<InstructionSequence> ControlFlowGraph::create_instruction_sequence() const {
  // There are two algorithms for creating the result InstructionSequence.
  // The ideal one is rebuild_instruction_sequence(), which uses the original
  // "code order" to sequence the basic blocks. This is desirable, because
  // it means that CFG transformations don't introduce unnecessary
  // resequencing of blocks, which makes the results of transformations easier
  // to compare with the original code. However, if control flow is changed,
  // reconstruct_instruction_sequence() will do a more "invasive" reconstruction
  // which guarantees that blocks connected by fall-through edges are placed
  // together, but has the disadvantage that it could arbitrarily reorder
  // blocks compared to their original order.

  if (can_use_original_block_order())
    return rebuild_instruction_sequence();
  else
    return reconstruct_instruction_sequence();
}

std::vector<const BasicBlock *> ControlFlowGraph::get_blocks_in_code_order() const {
  std::vector<const BasicBlock *> blocks_in_code_order;
  std::copy(m_basic_blocks.begin(), m_basic_blocks.end(), std::back_inserter(blocks_in_code_order));

  // Sort blocks by their code order
  std::sort(blocks_in_code_order.begin(), blocks_in_code_order.end(),
            [](const BasicBlock *left, const BasicBlock *right) {
              assert(left->get_code_order() != right->get_code_order());
              return left->get_code_order() < right->get_code_order();
            });

  return blocks_in_code_order;
}

bool ControlFlowGraph::can_use_original_block_order() const {
  // The result InstructionSequence can use the original block order
  // (the "code order") as long as every fall-through branch connects
  // to the block that is the successor in code order.

  std::vector<const BasicBlock *> blocks_in_code_order = get_blocks_in_code_order();

  // Check whether each fall-through edge leads to the next block
  // in code order
  bool good = true;
  auto i = blocks_in_code_order.begin();
  assert(i != blocks_in_code_order.end());
  const BasicBlock *cur = *i;
  ++i;
  while (good && i != blocks_in_code_order.end()) {
    // Get the next block in code order
    const BasicBlock *next = *i;
    ++i;

    // Check the edges of the current block to see if there is
    // a fall-through edge
    const EdgeList &outgoing_edges = get_outgoing_edges(cur);
    for (auto j = outgoing_edges.begin(); j != outgoing_edges.end(); ++j) {
      const Edge *edge = *j;
      if (edge->get_kind() == EDGE_FALLTHROUGH) {
        // If the fall-through edge doesn't lead to the next block
        // in code order, then we can't use the original block order
        if (edge->get_target() != next)
          good = false;
      }
    }

    // Continue to next pair of blocks
    cur = next;
  }

  return good;
}

std::shared_ptr<InstructionSequence> ControlFlowGraph::rebuild_instruction_sequence() const {
  // Rebuild an InstructionSequence in which the basic blocks are
  // placed in their original code order.  This should only be
  // done if can_use_original_block_order() returns true.

  std::shared_ptr<InstructionSequence> result(new InstructionSequence());

  std::vector<const BasicBlock *> blocks_in_code_order = get_blocks_in_code_order();
  std::vector<bool> finished_blocks(get_num_blocks(), false);

  for (auto i = blocks_in_code_order.begin(); i != blocks_in_code_order.end(); ++i) {
    const BasicBlock *bb = *i;
    append_basic_block(result, bb, finished_blocks);
  }

  return result;
}

std::shared_ptr<InstructionSequence> ControlFlowGraph::reconstruct_instruction_sequence() const {
  // Reconstruct an InstructionSequence when it's the case that the blocks
  // can not be laid out in the original code order because the control
  // flow has changed. This code does work (it was used in Fall 2020 and
  // Fall 2021 without issues.) However, it has the disadvantage of
  // sometimes making arbitrary changes to the order of the basic blocks,
  // which makes the transformed code harder to understand.

  // In theory, transformations that don't change control flow edges
  // should always permit the InstructionSequence to be laid out following
  // the code order of the original basic blocks (which is what
  // rebuild_instruction_sequence() does.)

  assert(m_entry != nullptr);
  assert(m_exit != nullptr);
  assert(m_outgoing_edges.size() == m_incoming_edges.size());

  // Find all Chunks (groups of basic blocks connected via fall-through)
  typedef std::map<BasicBlock *, Chunk *> ChunkMap;
  ChunkMap chunk_map;
  for (auto i = m_outgoing_edges.cbegin(); i != m_outgoing_edges.cend(); i++) {
    const EdgeList &outgoing_edges = i->second;
    for (auto j = outgoing_edges.cbegin(); j != outgoing_edges.cend(); j++) {
      Edge *e = *j;

      if (e->get_kind() != EDGE_FALLTHROUGH) {
        continue;
      }

      BasicBlock *pred = e->get_source();
      BasicBlock *succ = e->get_target();

      Chunk *pred_chunk = (chunk_map.find(pred) == chunk_map.end()) ? nullptr : chunk_map[pred];
      Chunk *succ_chunk = (chunk_map.find(succ) == chunk_map.end()) ? nullptr : chunk_map[succ];

      if (pred_chunk == nullptr && succ_chunk == nullptr) {
        // create a new chunk
        Chunk *chunk = new Chunk();
        chunk->append(pred);
        chunk->append(succ);
        chunk_map[pred] = chunk;
        chunk_map[succ] = chunk;
      } else if (pred_chunk == nullptr) {
        // prepend predecessor to successor's chunk (successor should be the first block)
        assert(succ_chunk->is_first(succ));
        succ_chunk->prepend(pred);
        chunk_map[pred] = succ_chunk;
      } else if (succ_chunk == nullptr) {
        // append successor to predecessor's chunk (predecessor should be the last block)
        assert(pred_chunk->is_last(pred));
        pred_chunk->append(succ);
        chunk_map[succ] = pred_chunk;
      } else {
        // merge the chunks
        Chunk *merged = pred_chunk->merge_with(succ_chunk);
        // update every basic block to point to the merged chunk
        for (auto i = merged->blocks.begin(); i != merged->blocks.end(); i++) {
          BasicBlock *bb = *i;
          chunk_map[bb] = merged;
        }
        // delete old Chunks
        delete pred_chunk;
        delete succ_chunk;
      }
    }
  }

  std::shared_ptr<InstructionSequence> result(new InstructionSequence());
  std::vector<bool> finished_blocks(get_num_blocks(), false);
  Chunk *exit_chunk = nullptr;

  // Traverse the CFG, appending basic blocks to the generated InstructionSequence.
  // If we find a block that is part of a Chunk, the entire Chunk is emitted.
  // (This allows fall through to work.)
  std::deque<BasicBlock *> work_list;
  work_list.push_back(m_entry);

  while (!work_list.empty()) {
    BasicBlock *bb = work_list.front();
    work_list.pop_front();
    unsigned block_id = bb->get_id();

    if (finished_blocks[block_id]) {
      // already added this block
      continue;
    }

    ChunkMap::iterator i = chunk_map.find(bb);
    if (i != chunk_map.end()) {
      // This basic block is part of a Chunk: append all of its blocks
      Chunk *chunk = i->second;

      // If this chunk contains the exit block, it needs to be at the end
      // of the generated InstructionSequence, so defer appending any of
      // its blocks.  (But, *do* find its control successors.)
      bool is_exit_chunk = false;
      if (chunk->contains_exit_block()) {
        exit_chunk = chunk;
        is_exit_chunk = true;
      }

      for (auto j = chunk->blocks.begin(); j != chunk->blocks.end(); j++) {
        BasicBlock *b = *j;
        if (is_exit_chunk) {
          // mark the block as finished, but don't append its instructions yet
          finished_blocks[b->get_id()] = true;
        } else {
          append_basic_block(result, b, finished_blocks);
        }

        // Visit control successors
        visit_successors(b, work_list);
      }
    } else {
      // This basic block is not part of a Chunk
      append_basic_block(result, bb, finished_blocks);

      // Visit control successors
      visit_successors(bb, work_list);
    }
  }

  // append exit chunk
  if (exit_chunk != nullptr) {
    append_chunk(result, exit_chunk, finished_blocks);
  }

  return result;
}

void ControlFlowGraph::append_basic_block(const std::shared_ptr<InstructionSequence> &iseq, const BasicBlock *bb, std::vector<bool> &finished_blocks) const {
  if (bb->has_label()) {
    iseq->define_label(bb->get_label());
  }
  for (auto i = bb->cbegin(); i != bb->cend(); i++) {
    iseq->append((*i)->duplicate());
  }
  finished_blocks[bb->get_id()] = true;
}

void ControlFlowGraph::append_chunk(const std::shared_ptr<InstructionSequence> &iseq, Chunk *chunk, std::vector<bool> &finished_blocks) const {
  for (auto i = chunk->blocks.begin(); i != chunk->blocks.end(); i++) {
    append_basic_block(iseq, *i, finished_blocks);
  }
}

void ControlFlowGraph::visit_successors(BasicBlock *bb, std::deque<BasicBlock *> &work_list) const {
  const EdgeList &outgoing_edges = get_outgoing_edges(bb);
  for (auto k = outgoing_edges.cbegin(); k != outgoing_edges.cend(); k++) {
    Edge *e = *k;
    work_list.push_back(e->get_target());
  }
}

void ControlFlowGraph::delete_blocks() {
  for (auto i = m_basic_blocks.begin(); i != m_basic_blocks.end(); ++i) {
    delete *i;
  }
}

void ControlFlowGraph::delete_edges(EdgeMap &edge_map) {
  for (auto i = edge_map.begin(); i != edge_map.end(); ++i) {
    for (auto j = i->second.begin(); j != i->second.end(); ++j) {
      delete *j;
    }
  }
}

////////////////////////////////////////////////////////////////////////
// ControlFlowGraphBuilder implementation
////////////////////////////////////////////////////////////////////////

ControlFlowGraphBuilder::ControlFlowGraphBuilder(const std::shared_ptr<InstructionSequence> &iseq)
  : m_iseq(iseq)
  , m_cfg(new ControlFlowGraph()) {
}

ControlFlowGraphBuilder::~ControlFlowGraphBuilder() {
}

std::shared_ptr<ControlFlowGraph> ControlFlowGraphBuilder::build() {
  unsigned num_instructions = m_iseq->get_length();

  BasicBlock *entry = m_cfg->create_basic_block(BASICBLOCK_ENTRY, -1);
  BasicBlock *exit = m_cfg->create_basic_block(BASICBLOCK_EXIT, 2000000);

  // exit block is reached by any branch that targets the end of the
  // InstructionSequence
  m_basic_blocks[num_instructions] = exit;

  std::deque<WorkItem> work_list;
  work_list.push_back({ ins_index: 0, pred: entry, edge_kind: EDGE_FALLTHROUGH });

  BasicBlock *last = nullptr;
  while (!work_list.empty()) {
    WorkItem item = work_list.front();
    work_list.pop_front();

    assert(item.ins_index <= m_iseq->get_length());

    // if item target is end of InstructionSequence, then it targets the exit block
    if (item.ins_index == m_iseq->get_length()) {
      m_cfg->create_edge(item.pred, exit, item.edge_kind);
      continue;
    }

    BasicBlock *bb;
    bool is_new_block;
    std::map<unsigned, BasicBlock *>::iterator i = m_basic_blocks.find(item.ins_index);
    if (i != m_basic_blocks.end()) {
      // a block starting at this instruction already exists
      bb = i->second;
      is_new_block = false;

      // Special case: if this block was originally discovered via a fall-through
      // edge, but is also reachable via a branch, then it might not be labeled
      // yet.  Set the label if necessary.
      if (item.edge_kind == EDGE_BRANCH && !bb->has_label()) {
        bb->set_label(item.label);
      }
    } else {
      // no block starting at this instruction currently exists:
      // scan the basic block and add it to the map of known basic blocks
      // (indexed by instruction index)
      bb = scan_basic_block(item, item.label);
      is_new_block = true;
      m_basic_blocks[item.ins_index] = bb;
    }

    // if the edge is a branch, make sure the work item's label matches
    // the BasicBlock's label (if it doesn't, then somehow this block
    // is reachable via two different labels, which shouldn't be possible)
    assert(item.edge_kind != EDGE_BRANCH || bb->get_label() == item.label);

    // connect to predecessor
    m_cfg->create_edge(item.pred, bb, item.edge_kind);

    if (!is_new_block) {
      // we've seen this block already
      continue;
    }

    // if this basic block ends in a branch, prepare to create an edge
    // to the BasicBlock for the target (creating the BasicBlock if it
    // doesn't exist yet)
    if (ends_in_branch(bb)) {
      unsigned target_index = get_branch_target_index(bb);
      // Note: we assume that branch instructions have the target label
      // as the last operand
      Instruction *branch = bb->get_last_instruction();
      unsigned num_operands = branch->get_num_operands();
      assert(num_operands > 0);
      Operand operand = branch->get_operand(num_operands - 1);
      assert(operand.get_kind() == Operand::LABEL);
      std::string target_label = operand.get_label();
      work_list.push_back({ ins_index: target_index, pred: bb, edge_kind: EDGE_BRANCH, label: target_label });
    }

    // if this basic block falls through, prepare to create an edge
    // to the BasicBlock for successor instruction (creating it if it doesn't
    // exist yet)
    if (falls_through(bb)) {
      unsigned target_index = item.ins_index + bb->get_length();
      assert(target_index <= m_iseq->get_length());
      if (target_index == num_instructions) {
        // this is the basic block at the end of the instruction sequence,
        // its fall-through successor should be the exit block
        last = bb;
      } else {
        // fall through to basic block starting at successor instruction
        work_list.push_back({ ins_index: target_index, pred: bb, edge_kind: EDGE_FALLTHROUGH });
      }
    }
  }

  assert(last != nullptr);
  m_cfg->create_edge(last, exit, EDGE_FALLTHROUGH);

  return m_cfg;
}

// Note that subclasses may override this method.
bool ControlFlowGraphBuilder::is_branch(Instruction *ins) {
  // assume that if an instruction's last operand is a label,
  // it's a branch instruction
  unsigned num_operands = ins->get_num_operands();
  return num_operands > 0 && ins->get_operand(num_operands - 1).get_kind() == Operand::LABEL;
}

BasicBlock *ControlFlowGraphBuilder::scan_basic_block(const WorkItem &item, const std::string &label) {
  unsigned index = item.ins_index;

  BasicBlock *bb = m_cfg->create_basic_block(BASICBLOCK_INTERIOR, int(index), label);

  // keep adding instructions until we
  // - reach an instruction that is a branch
  // - reach an instruction that is a target of a branch
  // - reach the end of the overall instruction sequence
  while (index < m_iseq->get_length()) {
    Instruction *ins = m_iseq->get_instruction(index);

    // this instruction is part of the basic block
    ins = ins->duplicate();
    bb->append(ins);
    index++;

    if (index >= m_iseq->get_length()) {
      // At end of overall instruction sequence
      break;
    }

    if (is_function_call(ins)) {
      // The instruction we just added is a function call
      break;
    }

    if (is_branch(ins)) {
      // The instruction we just added is a branch instruction
      break;
    }

    if (m_iseq->has_label(index)) {
      // Next instruction has a label, so assume it is a branch target
      // (and thus the beginning of a different basic block)
      break;
    }
  }

  assert(bb->get_length() > 0);

  return bb;
}

bool ControlFlowGraphBuilder::ends_in_branch(BasicBlock *bb) {
  return !is_function_call(bb->get_last_instruction()) && is_branch(bb->get_last_instruction());
}

unsigned ControlFlowGraphBuilder::get_branch_target_index(BasicBlock *bb) {
  assert(ends_in_branch(bb));
  Instruction *last = bb->get_last_instruction();

  // assume that the label is the last operand
  unsigned num_operands = last->get_num_operands();
  assert(num_operands > 0);
  Operand label = last->get_operand(num_operands - 1);
  assert(label.get_kind() == Operand::LABEL);

  // look up the index of the instruction targeted by this label
  unsigned target_index = m_iseq->get_index_of_labeled_instruction(label.get_label());
  return target_index;
}

bool ControlFlowGraphBuilder::falls_through(BasicBlock *bb) {
  return falls_through(bb->get_last_instruction());
}

////////////////////////////////////////////////////////////////////////
// HighLevelControlFlowGraphBuilder implementation
////////////////////////////////////////////////////////////////////////

HighLevelControlFlowGraphBuilder::HighLevelControlFlowGraphBuilder(const std::shared_ptr<InstructionSequence> &iseq)
  : ControlFlowGraphBuilder(iseq) {
}

HighLevelControlFlowGraphBuilder::~HighLevelControlFlowGraphBuilder() {
}

bool HighLevelControlFlowGraphBuilder::is_function_call(Instruction *ins) {
  return ins->get_opcode() == HINS_call;
}

bool HighLevelControlFlowGraphBuilder::falls_through(Instruction *ins) {
  // only an unconditional jump instruction does not fall through
  return ins->get_opcode() != HINS_jmp;
}

////////////////////////////////////////////////////////////////////////
// LowLevelControlFlowGraphBuilder implementation
////////////////////////////////////////////////////////////////////////

LowLevelControlFlowGraphBuilder::LowLevelControlFlowGraphBuilder(const std::shared_ptr<InstructionSequence> &iseq)
  : ControlFlowGraphBuilder(iseq) {
}

LowLevelControlFlowGraphBuilder::~LowLevelControlFlowGraphBuilder() {
}

bool LowLevelControlFlowGraphBuilder::is_function_call(Instruction *ins) {
  return ins->get_opcode() == MINS_CALL;
}

bool LowLevelControlFlowGraphBuilder::falls_through(Instruction *ins) {
  // only an unconditional jump instruction does not fall through
  return ins->get_opcode() != MINS_JMP;
}

////////////////////////////////////////////////////////////////////////
// ControlFlowGraphPrinter implementation
////////////////////////////////////////////////////////////////////////

ControlFlowGraphPrinter::ControlFlowGraphPrinter(const std::shared_ptr<ControlFlowGraph> &cfg)
  : m_cfg(cfg) {
}

ControlFlowGraphPrinter::~ControlFlowGraphPrinter() {
}

void ControlFlowGraphPrinter::print() {
  for (auto i = m_cfg->bb_begin(); i != m_cfg->bb_end(); i++) {
    BasicBlock *bb = *i;

    // Print information about the beginning of the basic block
    std::string begin_str;
    begin_str += "BASIC BLOCK ";
    begin_str += std::to_string(bb->get_id());
    BasicBlockKind bb_kind = bb->get_kind();
    if (bb_kind != BASICBLOCK_INTERIOR)
      begin_str += (bb_kind == BASICBLOCK_ENTRY ? " [entry]" : " [exit]");
    if (bb->has_label()) {
      begin_str += " (label ";
      begin_str += bb->get_label();
      begin_str += ")";
    }
    std::string begin_annotation = get_block_begin_annotation(bb);
    if (!begin_annotation.empty()) {
      if (begin_str.size() < 37)
        begin_str += ("                                     " + begin_str.size());
      begin_str += "/* ";
      begin_str += begin_annotation;
      begin_str += " */";
    }
    printf("%s\n", begin_str.c_str());

    // Print the instructions in the basic block
    print_basic_block(bb);

    // Print information about the outgoing edges
    const ControlFlowGraph::EdgeList &outgoing_edges = m_cfg->get_outgoing_edges(bb);
    for (auto j = outgoing_edges.cbegin(); j != outgoing_edges.cend(); j++) {
      const Edge *e = *j;
      assert(e->get_kind() == EDGE_BRANCH || e->get_kind() == EDGE_FALLTHROUGH);
      printf("  %s EDGE to BASIC BLOCK %u\n", e->get_kind() == EDGE_FALLTHROUGH ? "fall-through" : "branch", e->get_target()->get_id());
    }

    // If there is an end annotation, print it
    std::string end_annotation = get_block_end_annotation(bb);
    if (!end_annotation.empty())
      printf("                    At end of block: /* %s */\n", end_annotation.c_str());

    printf("\n");
  }
}

std::string ControlFlowGraphPrinter::get_block_begin_annotation(BasicBlock *bb) {
  return "";
}

std::string ControlFlowGraphPrinter::get_block_end_annotation(BasicBlock *bb) {
  return "";
}

////////////////////////////////////////////////////////////////////////
// HighLevelControlFlowGraphPrinter implementation
////////////////////////////////////////////////////////////////////////

HighLevelControlFlowGraphPrinter::HighLevelControlFlowGraphPrinter(const std::shared_ptr<ControlFlowGraph> &cfg)
  : ControlFlowGraphPrinter(cfg) {
}

HighLevelControlFlowGraphPrinter::~HighLevelControlFlowGraphPrinter() {
}

void HighLevelControlFlowGraphPrinter::print_basic_block(BasicBlock *bb) {
  HighLevelFormatter hl_formatter;
  PrintInstructionSequence print_iseq(&hl_formatter);
  print_iseq.print(bb);
}

////////////////////////////////////////////////////////////////////////
// LowLevelControlFlowGraphPrinter implementation
////////////////////////////////////////////////////////////////////////

LowLevelControlFlowGraphPrinter::LowLevelControlFlowGraphPrinter(const std::shared_ptr<ControlFlowGraph> &cfg)
  : ControlFlowGraphPrinter(cfg) {
}

LowLevelControlFlowGraphPrinter::~LowLevelControlFlowGraphPrinter() {
}

void LowLevelControlFlowGraphPrinter::print_basic_block(BasicBlock *bb) {
  LowLevelFormatter ll_formatter;
  PrintInstructionSequence print_iseq(&ll_formatter);
  print_iseq.print(bb);
}
