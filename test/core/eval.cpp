#include <tori/core.hpp>
#include <tori/lib.hpp>

#include <catch2/catch.hpp>

#include <iostream>

using namespace tori;

TEST_CASE("exception")
{
  SECTION("Exception return")
  {
    auto unit = make_object<Unit>();
    SECTION("bad_value_cast")
    {
      struct F : Function<F, Unit, Unit>
      {
        return_type code() const
        {
          object_ptr<> i = make_object<Int>();
          // throw bad_value_cast
          auto d = value_cast<Double>(i);
        }
      };

      [&]() {
        auto f = make_object<F>();
        try {
          eval(f << unit);
        } catch (const result_error::exception_result& e) {
          INFO(e.what());
          if (
            auto err =
              value_cast_if<BadValueCast>(e.exception()->error_value)) {
            INFO(
              "bad_value_cast: from=" << to_string(err->from)
                                      << " to=" << to_string(err->to));
            REQUIRE(true);
            return;
          }
        }
        REQUIRE(false);
      }();
    }

    SECTION("type_error")
    {
      struct F : Function<F, Unit, Unit>
      {
        return_type code() const
        {
          auto i = make_object<Int>();
          auto d = make_object<Double>();
          auto app = i << d;
          // throw type_missmatch
          auto t = type_of(app);
        }
      };

      [&]() {
        auto f = make_object<F>();
        try {
          eval(f << unit);
        } catch (const result_error::exception_result& e) {
          if (auto err = value_cast_if<TypeError>(e.exception()->error_value)) {
            switch (err->error_type) {
            case type_error_type::type_missmatch:
              std::cout << "type_error::type_missmatch" << std::endl;
              std::cout << "expected=" << to_string(err->expected)
                        << ",provided=" << to_string(err->provided)
                        << std::endl;
              REQUIRE(true);
              return;
            default:
              break;
            }
          }
        }
        REQUIRE(false);
      }();
    }

    SECTION("result_error")
    {
      struct F : Function<F, Unit, Unit>
      {
        return_type code() const
        {
          throw std::runtime_error("test message");
        }
      };

      struct G : Function<G, Unit, Unit>
      {
        return_type code() const
        {
          return eval_arg<0>();
        }
      };

      [&]() {
        auto f = make_object<F>();
        auto g = make_object<G>();

        try {
          eval(g << (f << unit));
        } catch (result_error::exception_result& e) {
          std::cout << (e.exception()->message->c_str()) << std::endl;
          REQUIRE(true);
          return;
        }
        REQUIRE(false);
      }();
    }
  }
}