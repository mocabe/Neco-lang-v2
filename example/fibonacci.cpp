#include <tori/core.hpp>
#include <tori/lib.hpp>
#include <iostream>
// just for this example...
using namespace tori;

struct fib : Function<fib, Int, Int> {
  struct impl : Function<impl, closure<Int, Int>, Int, Int> {
    return_type code() const
    {
      auto fib = arg<0>();
      auto n = eval_arg<1>();

      if (*n < 2)
        return n;

      auto l = eval(fib << new Int(*n - 1));
      auto r = eval(fib << new Int(*n - 2));
      return new Int(*value_cast<Int>(l) + *value_cast<Int>(r));
    }
  };
  return_type code() const
  {
    static const auto fix = make_object<Fix>();
    return fix << new impl() << arg<0>();
  }
};

int main()
{
  auto app = make_object<fib>() << new Int(10);
  try {
    check_type<Int>(app);
    auto result = eval(app);
    std::cout << *value_cast<Int>(result) << std::endl;
  } catch (std::exception& e) {
    // blah blah
    std::cout << e.what() << std::endl;
  }
}