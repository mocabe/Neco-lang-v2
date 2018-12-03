#include <tori/core.hpp>
#include <tori/lib.hpp>

#include <iostream>
// just for this example...
using namespace tori;

// Bool -> X -> X -> X
struct If
  : Function<::If, Bool, forall<class X>, forall<class X>, forall<class X>> {
  return_type code() const
  {
    if (*eval_arg<0>())
      return arg<1>();
    else
      return arg<2>();
  }
};

int main()
{
  // create objects
  auto if_ = make_object<::If>();
  auto b = make_object<Bool>(true);
  auto i1 = make_object<Int>(42);
  auto i2 = make_object<Int>(24);

  // apply
  auto app = if_ << b << i1 << i2;
  // check type
  check_type<Int>(app);
  // result
  auto result = eval(app);
  assert(*value_cast<Int>(result) == 42);
}