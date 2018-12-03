#include <tori/core.hpp>
#include <tori/lib.hpp>

#include <iostream>
// just for this example...
using namespace tori;

// Apply second argument twice to first argument
struct ApplyTwice : Function<ApplyTwice, closure<Int, Int>, Int, Int> {
  return_type code() const
  {
    // arg<1> (arg<0> arg<1>)
    return arg<0>() << (arg<0>() << arg<1>());
  }
};

struct X2 : Function<X2, Int, Int> {
  return_type code() const
  {
    // use eval_arg<0>() instead of arg<0>()
    // to evaluate argument.
    return new Int(*eval_arg<0>() * 2);
  }
};

int main()
{
  // create objects
  auto applyTwice = make_object<ApplyTwice>();
  auto func = make_object<X2>();
  // apply
  auto apply = applyTwice << func << new Int(42);
  // type check
  check_type<Int>(apply);
  // evaluate
  auto result = eval(apply);

  assert(*value_cast<Int>(result) == 168);
}