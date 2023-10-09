#include <cassert>
#include "instruction.h"
#include "exceptions.h"
#include "instruction_seq.h"

InstructionSequence::InstructionSequence()
  : m_funcdef_ast(nullptr) {
}

InstructionSequence::~InstructionSequence() {
  // delete the Instructions
  for (auto i = m_instructions.begin(); i != m_instructions.end(); ++i)
    delete i->ins;
}

InstructionSequence *InstructionSequence::duplicate() const {
  InstructionSequence *dup = new InstructionSequence();
  dup->m_next_label = m_next_label;
  dup->m_funcdef_ast = m_funcdef_ast;

  for (auto i = m_instructions.begin(); i != m_instructions.end(); ++i) {
    const Slot &slot = *i;
    if (!slot.label.empty())
      dup->define_label(slot.label);
    dup->append(slot.ins->duplicate());
  }

  return dup;
}

void InstructionSequence::append(Instruction *ins) {
  if (!m_next_label.empty()) {
    unsigned index = unsigned(m_instructions.size());
    m_label_map[m_next_label] = index;
  }
  m_instructions.push_back({ label: m_next_label, ins: ins });
  m_next_label = "";
}

unsigned InstructionSequence::get_length() const {
  return unsigned(m_instructions.size());
}

Instruction *InstructionSequence::get_instruction(unsigned index) const {
  return m_instructions.at(index).ins;
}

Instruction *InstructionSequence::get_last_instruction() const {
  assert(!m_instructions.empty());
  return m_instructions.back().ins;
}

void InstructionSequence::define_label(const std::string &label) {
  assert(m_next_label.empty());
  m_next_label = label;
}

bool InstructionSequence::has_label_at_end() const {
  return !m_next_label.empty();
}

InstructionSequence::const_iterator InstructionSequence::get_iterator_at_labeled_position(const std::string &label) const {
  auto i = m_label_map.find(label);
  if (i == m_label_map.end())
    return const_iterator(m_instructions.end());
  else
    return const_iterator(m_instructions.begin() + i->second);
}

Instruction *InstructionSequence::find_labeled_instruction(const std::string &label) const {
  const_iterator i = get_iterator_at_labeled_position(label);
  return (i != cend()) ? *i : nullptr;
}

unsigned InstructionSequence::get_index_of_labeled_instruction(const std::string &label) const {
  auto i = get_iterator_at_labeled_position(label);
  if (i == cend())
    RuntimeError::raise("no instruction has label '%s'", label.c_str());
  return unsigned(i - cbegin());
}
