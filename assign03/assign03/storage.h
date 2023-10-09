#ifndef STORAGE_H
#define STORAGE_H

#include "type.h"

// Compute storage size and field offsets for
// struct and union types. This can *also* be used to
// compute offsets and total storage size for local variables.
// Laying out fields in a struct and laying out storage for variables
// in a stack frame is essentially the same problem.
class StorageCalculator {
public:
  enum Mode { STRUCT, UNION };

private:
  Mode m_mode;
  unsigned m_size;
  unsigned m_align;
  bool m_finished;

public:
  // Note that the start_offset parameter is useful if you are
  // computing storage for local variables in a nested scope.
  // (You can set it to be the total amount of storage used
  // by the outer scopes.)
  StorageCalculator(Mode mode = STRUCT, unsigned start_offset = 0);
  ~StorageCalculator();

  // Add a field of given type.
  // Returns the field's storage offset.
  unsigned add_field(const std::shared_ptr<Type> &type);

  // Call this after all fields have been added.
  // Adds padding at end (if necessary).
  void finish();

  // Get storage size of overall struct or union
  unsigned get_size() const;

  // Get storage alignment of overall struct or union
  unsigned get_align() const;
};

#endif // STORAGE_H
