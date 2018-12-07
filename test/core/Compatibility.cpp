#include <tori/core.hpp>

void comp()
{
  using namespace tori;
  using namespace tori::detail;

  // ----------------------------------------
  // HeapObject
  // ----------------------------------------
  // no padding
  static_assert(sizeof(HeapObject) == 16 + object_header_extend_bytes);
  static_assert(sizeof(object_info_table) == 40);

  // ----------------------------------------
  // object_ptr
  // ----------------------------------------
  static_assert(sizeof(object_ptr<>) == 8);

  // ----------------------------------------
  // Type
  // ----------------------------------------
  static_assert(sizeof(ValueType) == 8);
  static_assert(sizeof(ArrowType) == 16);
  static_assert(sizeof(VarType) == 8);
  static_assert(sizeof(TypeValue) == 24);

  // ----------------------------------------
  // Closure
  // ----------------------------------------
  // no padding
  static_assert(
    sizeof(ClosureN<4>) ==
    sizeof(HeapObject) + 8 + closure_header_extend_bytes + 4 * 8);
  static_assert(sizeof(closure_info_table) == sizeof(object_info_table) + 24);

  // ----------------------------------------
  // Object
  // ----------------------------------------
  static_assert(sizeof(expected<Fix>) == sizeof(HeapObject));
  static_assert(sizeof(closure<Fix, Fix>) == sizeof(HeapObject));
  static_assert(sizeof(forall<void>) == sizeof(HeapObject));

  static_assert(sizeof(ExceptionValue) == 8);
  static_assert(
    sizeof(Exception) == sizeof(ExceptionValue) + sizeof(HeapObject));

  static_assert(sizeof(TypeErrorValue) == 16);
  static_assert(
    sizeof(TypeError) == sizeof(TypeErrorValue) + sizeof(HeapObject));

  static_assert(sizeof(EvalErrorValue) == 16);
  static_assert(
    sizeof(EvalError) == sizeof(EvalErrorValue) + sizeof(HeapObject));

  // strict size of Fix is not actually required
  static_assert(sizeof(FixValue) == 1);
  static_assert(sizeof(Fix) == 8 + sizeof(HeapObject));

  static_assert(sizeof(ApplyRValue) == 16);
  static_assert(sizeof(ApplyR) == sizeof(HeapObject) + sizeof(ApplyRValue));
  static_assert(sizeof(ApplyR) == sizeof(Apply<Fix, Fix>));
}

int main()
{
  comp();
}