#include <tori/core.hpp>
#include <tori/lib.hpp>

#include <catch2/catch.hpp>

using namespace tori;

TEST_CASE("pointer construct")
{
  SECTION("pointer")
  {
    object_ptr<Int> i = nullptr;
    object_ptr<Int> j {};
    REQUIRE(i == nullptr);
    REQUIRE(j == nullptr);
    REQUIRE(nullptr == i);
  }
  SECTION("pointer")
  {
    object_ptr<Int> i = new Int(42);
    REQUIRE(i);
    REQUIRE(*i == 42);
  }
  SECTION("deduction")
  {
    object_ptr i = new Int(42);
    REQUIRE(i);
    REQUIRE(*i == 42);
  }
  SECTION("deduction")
  {
    auto i = object_ptr(new Int(42));
    REQUIRE(i);
    REQUIRE(*i == 42);
  }
  SECTION("make_object")
  {
    auto i = make_object<Int>(42);
    REQUIRE(i);
    REQUIRE(*i == 42);
  }
  SECTION("copy")
  { 
    auto i = make_object<Int>(42);
    auto j = i;
    REQUIRE(j);
    REQUIRE(*j == 42);
  }
  SECTION("move")
  {
    auto i = make_object<Int>(42);
    auto j = std::move(i);
    REQUIRE(j);
    REQUIRE(*j == 42);
  }
  SECTION("pointer comparison")
  {
    auto i = make_object<Int>(42);
    auto j = i;
    REQUIRE(i == j);
    REQUIRE(!(i != j));
  }
  SECTION("conversion")
  {
    auto i = make_object<Int>(42);
    object_ptr<> j = i;
    auto k = object_ptr<>(i);
    REQUIRE(i == j);
    REQUIRE(!(i != j));
  }
}

TEST_CASE("immediate construct")
{
  SECTION("immediate")
  {
    immediate<int> i {};
    REQUIRE(!i);
    REQUIRE(*i == 0);
  }
  SECTION("immediate")
  {
    immediate<int> i = 42;
    REQUIRE(i);
    REQUIRE(*i == 42);
  }
  SECTION("deduction")
  {
    immediate i = 42;
    REQUIRE(i);
    REQUIRE(*i == 42);
  }
  SECTION("deduction")
  {
    auto i = immediate(42);
    REQUIRE(i);
    REQUIRE(*i == 42);
  }
  SECTION("make_object")
  {
    auto i = make_object<int>(42);
    REQUIRE(i);
    REQUIRE(*i == 42);
  }
  SECTION("copy")
  {
    auto i = immediate(42);
    auto j = i;
    REQUIRE(*j == 42);
  }
  SECTION("move")
  {
    auto i = immediate(42);
    auto j = std::move(i);
    REQUIRE(*j == 42);
  }
}

TEST_CASE("generic construct")
{
  SECTION("nullptr")
  {
    object_ptr_generic p = nullptr;
    REQUIRE(p.is_pointer());
  }
  SECTION("pointer")
  {
    object_ptr_generic p = new Int(42);
    REQUIRE(p.is_pointer());
  }
  SECTION("immediate")
  {
    object_ptr_generic p = 42;
    REQUIRE(p.is_immediate());
  }
  SECTION("copy")
  {
    auto i = object_ptr_generic(new Int(42));
    auto j = i;
    REQUIRE(i.is_pointer());
    REQUIRE(j.is_pointer());
  }
  SECTION("copy")
  {
    auto i = object_ptr_generic(42);
    auto j = i;
    REQUIRE(i.is_immediate());
    REQUIRE(j.is_immediate());
  }
  SECTION("move")
  {
    auto i = object_ptr_generic(new Int(42));
    auto j = std::move(i);
    REQUIRE(j.is_pointer());
  }
  SECTION("move")
  {
    auto i = object_ptr_generic(42);
    auto j = std::move(i);
    REQUIRE(j.is_immediate());
  }
  SECTION("generic pointer conversion")
  {
    auto i = make_object<Int>(42);
    object_ptr_generic g = i;
    auto p = value_cast<Int>(g);
    REQUIRE(*p == 42);
  }
  SECTION("generic immediate conversion")
  {
    auto i = make_object<int>(42);
    object_ptr_generic g = i;
    auto p = value_cast<int>(g);
    REQUIRE(*p == 42);
  }
}

TEST_CASE("operator bool")
{
  SECTION("immediate")
  {
    immediate<int> i {};
    REQUIRE(!i);
    *i = 42;
    REQUIRE(i);
    *i = 0;
    REQUIRE(!i);
  }

  SECTION("pointer")
  {
    object_ptr<Int> i {};
    REQUIRE(!i);
    i = new Int(42);
    REQUIRE(i);
    i = nullptr;
    REQUIRE(!i);
  }

  SECTION("generic")
  {
    object_ptr_generic g {};

    REQUIRE(!g);

    SECTION("immediate")
    {
      g = 42;
      REQUIRE(g);
      g = nullptr;
      REQUIRE(!g);
    }

    SECTION("pointer")
    {
      g = new Int(42);
      REQUIRE(g);
      g = 0;
      REQUIRE(!g);
    }
  }
}

TEST_CASE("value_cast")
{
  SECTION("immediate")
  {
    immediate<int> i = 42;
    object_ptr_generic g = i;

    SECTION("value_cast")
    {
      REQUIRE_NOTHROW(value_cast<int>(g));
      REQUIRE_THROWS_AS(value_cast<float>(g), bad_value_cast);
      REQUIRE_THROWS_AS(value_cast<Float>(g), bad_value_cast);
      REQUIRE(*value_cast<int>(g) == 42);
    }
  }

  SECTION("pointer")
  {
    object_ptr i = new Int(42);
    object_ptr_generic g = i;

    SECTION("value_cast")
    {
      REQUIRE_NOTHROW(value_cast<Int>(g));
      REQUIRE_THROWS_AS(value_cast<float>(g), bad_value_cast);
      REQUIRE_THROWS_AS(value_cast<Float>(g), bad_value_cast);
      REQUIRE(*value_cast<Int>(g) == 42);
    }

    SECTION("value_cast_if")
    {
      REQUIRE(value_cast_if<Int>(g));
      REQUIRE(!value_cast_if<Float>(g));
      REQUIRE(*value_cast_if<Int>(g) == 42);
    }
  }

  SECTION("obj")
  {
    object_ptr i = new Int(42);
    object_ptr<Object> o = i;

    SECTION("value_cast")
    {
      REQUIRE_NOTHROW(value_cast<Int>(o));
      REQUIRE_THROWS_AS(value_cast<Double>(o), bad_value_cast);
      REQUIRE(*value_cast<Int>(o) == 42);
    }
  }
}