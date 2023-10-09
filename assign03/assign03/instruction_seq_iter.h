#ifndef INSTRUCTION_SEQ_ITER_H
#define INSTRUCTION_SEQ_ITER_H

#include <cstddef> // for ptrdiff_t

class Instruction;

// Generic const_iterator type for InstructionSequence.
// When dereferenced, provides a pointer to the referenced Instruction
// object.  The has_label() and get_label() member functions can
// be used (respectively) to determine if the referenced Instruction
// has a label and to access the label.  It is parametized with the
// underlying vector const iterator type, to allow forward and reverse
// versions to be defined easily.  It supports random access
// (adding or subtracting a signed integer offset.)
template<typename It>
class ISeqIterator {
private:
  It slot_iter;

public:
  ISeqIterator() { }

  ISeqIterator(It i) : slot_iter(i) { }

  ISeqIterator(const ISeqIterator<It> &other) : slot_iter(other.slot_iter) { }

  ISeqIterator<It> &operator=(const ISeqIterator<It> &rhs) {
    if (this != &rhs) { slot_iter = rhs.slot_iter; }
    return *this;
  }

  // Equality and inequality comparisons

  bool operator==(const ISeqIterator<It> &rhs) const {
    return slot_iter == rhs.slot_iter;
  }

  bool operator!=(const ISeqIterator<It> &rhs) const {
    return slot_iter != rhs.slot_iter;
  }

  // Dereference
  Instruction* operator*() const { return slot_iter->ins; }

  // Access to the referenced Instruction's label
  bool has_label() const { return !slot_iter->label.empty(); }
  std::string get_label() const { return slot_iter->label; }

  // Increment and decrement

  ISeqIterator<It> &operator++() {
    ++slot_iter;
    return *this;
  }

  ISeqIterator<It> operator++(int) {
    ISeqIterator<It> copy(*this);
    ++slot_iter;
    return copy;
  }

  ISeqIterator<It> &operator--() {
    --slot_iter;
    return *this;
  }

  ISeqIterator<It> operator--(int) {
    ISeqIterator<It> copy(*this);
    --slot_iter;
    return copy;
  }

  // Support
  //   - adding and subtracting integer values
  //   - computing pointer difference between iterators
  //   - relational operators
  // so that instruction sequence iterators are random access

  ISeqIterator<It> operator+(ptrdiff_t i) const {
    return { slot_iter + i };
  }

  ISeqIterator<It> operator-(ptrdiff_t i) const {
    return { slot_iter - i };
  }

  ISeqIterator<It> &operator+=(ptrdiff_t i) {
    slot_iter += i;
    return *this;
  }

  ISeqIterator<It> &operator-=(ptrdiff_t i) {
    slot_iter -= i;
    return *this;
  }

  ptrdiff_t operator-(ISeqIterator<It> rhs) {
    return slot_iter - rhs.slot_iter;
  }

  bool operator<(ISeqIterator<It> rhs) const {
    return slot_iter < rhs.slot_iter;
  }

  bool operator<=(ISeqIterator<It> rhs) const {
    return slot_iter <= rhs.slot_iter;
  }

  bool operator>(ISeqIterator<It> rhs) const {
    return slot_iter > rhs.slot_iter;
  }

  bool operator>=(ISeqIterator<It> rhs) const {
    return slot_iter >= rhs.slot_iter;
  }
};

#endif // INSTRUCTION_SEQ_ITER_H
