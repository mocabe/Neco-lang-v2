#include <tori/core.hpp>
#include <tori/lib.hpp>

#include <iostream>
// just for this example...
using namespace tori;

// String -> Unit
struct Print : Function<Print, String, Unit> {
  ReturnType code() const {
    std::cout << eval_arg<0>()->c_str() << std::endl;
    // return type is checked in compile time
    return new Unit();
  }
};

int main() {
  // create closure
  auto print = make_object<Print>();
  // apply
  auto apply = print << new String("Hello, World!");
  // runtime type check
  check_type<Unit>(apply);
  // eval
  (void)eval(apply); // Hello, World!
}