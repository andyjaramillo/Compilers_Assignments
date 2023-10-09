#include "cfg.h"
#include "highlevel_formatter.h"
#include "lowlevel_formatter.h"
#include "live_vregs.h"
#include "live_mregs.h"
#include "print_instruction_seq.h"
#include "print_cfg.h"

////////////////////////////////////////////////////////////////////////
// PrintHighLevelCFG implementation
////////////////////////////////////////////////////////////////////////

PrintHighLevelCFG::PrintHighLevelCFG() {
}

PrintHighLevelCFG::~PrintHighLevelCFG() {
}

void PrintHighLevelCFG::print_instructions(const std::shared_ptr<InstructionSequence> &iseq) {
  HighLevelControlFlowGraphBuilder cfg_builder(iseq);

  std::shared_ptr<ControlFlowGraph> hl_cfg = cfg_builder.build();

  print_cfg(hl_cfg);
}

void PrintHighLevelCFG::print_cfg(const std::shared_ptr<ControlFlowGraph> &hl_cfg) {
  HighLevelControlFlowGraphPrinter hl_cfg_printer(hl_cfg);
  hl_cfg_printer.print();
}

////////////////////////////////////////////////////////////////////////
// Implementation of PrintInstructionSequence which annotates
// the formatted instructions with dataflow facts.
////////////////////////////////////////////////////////////////////////

template<typename Dataflow>
class PrintInstructionSequenceWithDataflowFacts : public PrintInstructionSequence {
private:
  Dataflow *m_dataflow;

public:
  PrintInstructionSequenceWithDataflowFacts(Formatter *formatter, Dataflow *liveness);
  virtual ~PrintInstructionSequenceWithDataflowFacts();

  virtual std::string get_instruction_annotation(const InstructionSequence *iseq, Instruction *ins);
};

template<typename Dataflow>
PrintInstructionSequenceWithDataflowFacts<Dataflow>::PrintInstructionSequenceWithDataflowFacts(Formatter *formatter, Dataflow *liveness)
  : PrintInstructionSequence(formatter)
  , m_dataflow(liveness) {
}

template<typename Dataflow>
PrintInstructionSequenceWithDataflowFacts<Dataflow>::~PrintInstructionSequenceWithDataflowFacts() {
}

template<typename Dataflow>
std::string PrintInstructionSequenceWithDataflowFacts<Dataflow>::get_instruction_annotation(const InstructionSequence *iseq, Instruction *ins) {
  // The InstructionSequence is actually a BasicBlock
  const BasicBlock *bb = static_cast<const BasicBlock *>(iseq);

  // get dataflow fact just before the instruction
  typename Dataflow::FactType fact = m_dataflow->get_fact_before_instruction(bb, ins);
  std::string s = Dataflow::fact_to_string(fact);
  //printf("annotation: %s\n", s.c_str());
  return s;
}

////////////////////////////////////////////////////////////////////////
// DataflowCFGPrinter implementation
//
// This class overrides either HighLevelControlFlowGraphPrinter
// or LowLevelControlFlowGraphPrinter, in order to use an
// implementation of PrintInstructionSequence that annotates each
// high-level instruction with computed dataflow facts.
////////////////////////////////////////////////////////////////////////

template<typename BaseClass, typename Dataflow, typename Formatter>
class DataflowCFGPrinter : public BaseClass {
private:
  Dataflow m_dataflow;

public:
  DataflowCFGPrinter(const std::shared_ptr<ControlFlowGraph> &cfg);
  virtual ~DataflowCFGPrinter();

  virtual std::string get_block_begin_annotation(BasicBlock *bb);
  virtual std::string get_block_end_annotation(BasicBlock *bb);

  virtual void print_basic_block(BasicBlock *bb);
};

template<typename BaseClass, typename Dataflow, typename Formatter>
DataflowCFGPrinter<BaseClass, Dataflow, Formatter>::DataflowCFGPrinter(const std::shared_ptr<ControlFlowGraph> &cfg)
  : BaseClass(cfg)
  , m_dataflow(cfg) {
  m_dataflow.execute();
}

template<typename BaseClass, typename Dataflow, typename Formatter>
DataflowCFGPrinter<BaseClass, Dataflow, Formatter>::~DataflowCFGPrinter() {
}

template<typename BaseClass, typename Dataflow, typename Formatter>
std::string DataflowCFGPrinter<BaseClass, Dataflow, Formatter>::get_block_begin_annotation(BasicBlock *bb) {
  typename Dataflow::FactType fact = m_dataflow.get_fact_at_beginning_of_block(bb);
  return Dataflow::fact_to_string(fact);
}

template<typename BaseClass, typename Dataflow, typename Formatter>
std::string DataflowCFGPrinter<BaseClass, Dataflow, Formatter>::get_block_end_annotation(BasicBlock *bb) {
  typename Dataflow::FactType fact = m_dataflow.get_fact_at_end_of_block(bb);
  return Dataflow::fact_to_string(fact);
}

template<typename BaseClass, typename Dataflow, typename Formatter>
void DataflowCFGPrinter<BaseClass, Dataflow, Formatter>::print_basic_block(BasicBlock *bb) {
  Formatter formatter;
  PrintInstructionSequenceWithDataflowFacts<Dataflow> print_iseq(&formatter, &m_dataflow);

  print_iseq.print(bb);
}

////////////////////////////////////////////////////////////////////////
// PrintHighLevelCFGWithLiveness implementation
////////////////////////////////////////////////////////////////////////

PrintHighLevelCFGWithLiveness::PrintHighLevelCFGWithLiveness() {
}

PrintHighLevelCFGWithLiveness::~PrintHighLevelCFGWithLiveness() {
}

void PrintHighLevelCFGWithLiveness::print_cfg(const std::shared_ptr<ControlFlowGraph> &hl_cfg) {
  DataflowCFGPrinter<HighLevelControlFlowGraphPrinter, LiveVregs, HighLevelFormatter> cfg_printer(hl_cfg);
  cfg_printer.print();
}

////////////////////////////////////////////////////////////////////////
// PrintLowLevelCFG implementation
////////////////////////////////////////////////////////////////////////

PrintLowLevelCFG::PrintLowLevelCFG() {
}

PrintLowLevelCFG::~PrintLowLevelCFG() {
}

void PrintLowLevelCFG::print_instructions(const std::shared_ptr<InstructionSequence> &iseq) {
  LowLevelControlFlowGraphBuilder ll_cfg_builder(iseq);
  std::shared_ptr<ControlFlowGraph> ll_cfg = ll_cfg_builder.build();
  print_cfg(ll_cfg);
}

void PrintLowLevelCFG::print_cfg(const std::shared_ptr<ControlFlowGraph> &cfg) {
  LowLevelControlFlowGraphPrinter ll_cfg_printer(cfg);
  ll_cfg_printer.print();
}

////////////////////////////////////////////////////////////////////////
// PrintLowLevelCFGWithLiveness implementation
////////////////////////////////////////////////////////////////////////

PrintLowLevelCFGWithLiveness::PrintLowLevelCFGWithLiveness() {
}

PrintLowLevelCFGWithLiveness::~PrintLowLevelCFGWithLiveness() {
}

void PrintLowLevelCFGWithLiveness::print_cfg(const std::shared_ptr<ControlFlowGraph> &cfg) {
  DataflowCFGPrinter<LowLevelControlFlowGraphPrinter, LiveMregs, LowLevelFormatter> cfg_printer(cfg);
  cfg_printer.print();
}
