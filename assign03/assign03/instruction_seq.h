#ifndef INSTRUCTION_SEQ_H
#define INSTRUCTION_SEQ_H

#include <vector>
#include <string>
#include <map>
#include "instruction_seq_iter.h"

class Instruction;
// FIXME: this needs to go away
class Node;

class InstructionSequence {
private:
  struct Slot {
    std::string label;
    Instruction *ins;
  };

  std::vector<Slot> m_instructions;
  std::map<std::string, unsigned> m_label_map; // label to instruction index map
  std::string m_next_label;
  // FIXME: this needs to go away
  Node *m_funcdef_ast; // pointer to function definition AST node

  // copy constructor and assignment operator are not allowed
  InstructionSequence(const InstructionSequence &);
  InstructionSequence &operator=(const InstructionSequence &);

public:
  // Iterator types, providing a pointer to an Instruction object when
  // dereferenced. These are random access.
  typedef ISeqIterator<std::vector<Slot>::const_iterator> const_iterator;
  typedef ISeqIterator<std::vector<Slot>::const_reverse_iterator> const_reverse_iterator;

  InstructionSequence();
  virtual ~InstructionSequence();

  // Return a dynamically-allocated duplicate of this InstructionSequence
  InstructionSequence *duplicate() const;

  // FIXME: this needs to go away
  // Access to function definition AST node
  void set_funcdef_ast(Node *funcdef_ast) { m_funcdef_ast = funcdef_ast; }
  Node *get_funcdef_ast() const { return m_funcdef_ast; }

  // get begin and end const_iterators
  const_iterator cbegin() const { return const_iterator(m_instructions.cbegin()); }
  const_iterator cend() const { return const_iterator(m_instructions.cend()); }

  // get begin and end const_reverse_iterators
  const_reverse_iterator crbegin() const { return const_reverse_iterator(m_instructions.crbegin()); }
  const_reverse_iterator crend() const { return const_reverse_iterator(m_instructions.crend()); }

  // apply a function to each Instruction in order
  template<typename Fn>
  void apply_to_all(Fn f) {
    for (auto i = cbegin(); i != cend(); ++i)
      f(*i);
  }

  // Append a pointer to an Instruction.
  // The InstructionSequence will assume responsibility for deleting the
  // Instruction object.
  void append(Instruction *ins);

  // Get number of Instructions.
  unsigned get_length() const;

  // Get Instruction at specified index.
  Instruction *get_instruction(unsigned index) const;

  // Get the last Instruction.
  Instruction *get_last_instruction() const;

  // Define a label. The next Instruction appended will be labeled with
  // this label.
  void define_label(const std::string &label);

  // Determine if Instruction at given index has a label.
  bool has_label(unsigned index) const { return !m_instructions.at(index).label.empty(); }

  // Get label at specified index
  std::string get_label_at_index(unsigned index) const { return m_instructions.at(index).label; }

  // Determine if Instruction referred to by specified iterator has a label.
  bool has_label(const_iterator i) const { return i.has_label(); }

  // Determine if the InstructionSequence has a label at the end
  bool has_label_at_end() const;

  // Return a forward const iterator positioned at the instruction with
  // the specified label, or the end iterator if there is no instruction
  // with the specified label
  const_iterator get_iterator_at_labeled_position(const std::string &label) const;

  // Find Instruction labeled with specified label.
  // Returns null pointer if no Instruction has the specified label.
  Instruction *find_labeled_instruction(const std::string &label) const;

  // Return the index of instruction labeled with the specified label.
  unsigned get_index_of_labeled_instruction(const std::string &label) const;
};

#endif // INSTRUCTION_SEQ_H
