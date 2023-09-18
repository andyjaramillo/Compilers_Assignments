#ifndef VALUE_H
#define VALUE_H

#include <cassert>
#include <string>
class ValRep;
class Function;

enum ValueKind {
  // "atomic" values
  VALUE_INT,
  VALUE_INTRINSIC_FN,
  // could other kinds of atomic values here

  // dynamic values: these have an associated dynamically-allocated
  // object (drived from ValRep)
  VALUE_FUNCTION,
  // could add other kinds of dynamic values here
};

// Typedef of the signature of an intrinsic function.
// Any information that intrinsic functions will need
// should be passed as parameters.
class Value;
class Location;
class Interpreter;
typedef Value (*IntrinsicFn)(Value args[], unsigned num_args, const Location &loc, Interpreter *interp);

// An "atomic" value that doesn't require a separate
// (dynamically-allocated) representation
union Atomic {
  int ival;
  IntrinsicFn intrinsic_fn;
};

// An instance of Value is a runtime value.
// Its type can vary (int, function, intrinsic function, etc.)

class Value {
private:
  ValueKind m_kind;
  union {
    Atomic m_atomic; // for "atomic" values
    ValRep *m_rep;   // for "dynamic" values (pointer to associated ValRep object)
  };

public:
  Value(int ival = 0);
  Value(Function *fn);
  Value(IntrinsicFn intrinsic_fn);
  Value(const Value &other);
  ~Value();

  Value &operator=(const Value &rhs);

  ValueKind get_kind() const { return m_kind; }

  // Getters to extract the contents of a Value.
  // The caller should use get_kind() first to determine
  // what kind of data the Value is storing.

  int get_ival() const {
    assert(m_kind == VALUE_INT);
    return m_atomic.ival;
  }

  Function *get_function() const;

  IntrinsicFn get_intrinsic_fn() const {
    assert(m_kind == VALUE_INTRINSIC_FN);
    return m_atomic.intrinsic_fn;
  }

  // convert to a string representation
  std::string as_str() const;

  bool is_numeric() const { return m_kind == VALUE_INT; }
  bool is_dynamic() const { return m_kind >= VALUE_FUNCTION; }
  bool is_atomic() const  { return !is_dynamic(); }

private:
  // TODO: add additional member functions, if necessary
};

#endif // VALUE_H
