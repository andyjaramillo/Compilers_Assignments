#include <cassert>
#include "storage.h"

namespace {

unsigned pad(unsigned offset, unsigned align) {
  // alignments must be powers of 2
  assert((align & (align - 1U)) == 0U);

  // Determine by how many bytes (if any) the offset is misaligned
  unsigned misalignment = offset & (align - 1U);

  // Determine how many bytes of padding should be added to the offset
  // to restore correct alignment
  assert(misalignment < align);
  return misalignment == 0U ? 0U : (align - misalignment);
}

}

StorageCalculator::StorageCalculator(Mode mode, unsigned start_offset)
  : m_mode(mode)
  , m_size(0U)
  , m_align(start_offset)
  , m_finished(false) {
  assert(mode == STRUCT || mode == UNION);
}

StorageCalculator::~StorageCalculator() {
}

unsigned StorageCalculator::add_field(const std::shared_ptr<Type> &type) {
  unsigned size = type->get_storage_size();
  unsigned align = type->get_alignment();

  // Keep track of largest alignment requirement, because
  // that will become the overall struct or union's
  // required alignment
  if (align > m_align) {
    m_align = align;
  }

  unsigned field_offset;

  if (m_mode == STRUCT) {
    // Determine amount of padding needed
    unsigned padding = pad(m_size, align);
    m_size += padding;

    // Now we know the offset of this field
    field_offset = m_size;

    // Next offset will, at a minimum, need to place the next
    // field beyond the storage for this one
    m_size += size;
  } else {
    // For a union, all field offsets are 0, and the union's
    // overall size is just the size of the largest member
    field_offset = 0U;
    if (size > m_size) {
      m_size = size;
    }
  }

  return field_offset;
}

void StorageCalculator::finish() {
  if (m_align == 0U) {
    // special case: if the struct or union has no fields,
    // its size is 0 and its alignment is 1
    assert(m_size == 0U);
    m_align = 1U;
  } else {
    if (m_mode == STRUCT) {
      // pad so that the overall size of the struct is a multiple
      // of the maximum field alignment
      m_size += pad(m_size, m_align);
    }
  }

  assert((m_align & (m_align - 1U)) == 0U);
  assert((m_size & (m_align - 1U)) == 0U);
  m_finished = true;
}

unsigned StorageCalculator::get_size() const {
  assert(m_finished);
  return m_size;
}

unsigned StorageCalculator::get_align() const {
  assert(m_finished);
  return m_align;
}
