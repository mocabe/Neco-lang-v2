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

// dummy
struct Func : Function<Func, Int, Int> {
  return_type code() const
  {
    std::cout << "Func called" << std::endl;
    return new Int(*eval_arg<0>() * 2);
  }
};

int main()
{
  auto applyTwice = make_object<ApplyTwice>();
  auto func = make_object<Func>();
  auto apply = applyTwice << func << new Int(42);
  check_type<Int>(apply);
  auto result = eval(apply);
  // Func called
  // Func called
  std::cout << *value_cast<Int>(result) << std::endl; // 168
}