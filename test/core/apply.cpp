#include <tori/core.hpp>
#include <tori/lib.hpp>

#include <catch2/catch.hpp>

using namespace tori;

TEST_CASE("apply")
{
  SECTION("pointer")
  {
    struct F : Function<F, closure<Int, Int>, Int, Int>
    {
      return_type code() const
      {
        return arg<0>() << arg<1>();
      }
    };
    auto f = make_object<F>();
  }

  SECTION("immediate")
  {
  }
}