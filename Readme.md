# Tori

**This library is still work in progress.**  
**...and not a serious project at all.**

---

`Tori` is light-weight, header-only functional plugin library written in C++17

## Overview
- Runtime typed closure object which supports lazy evaluation, currying, etc.
- Simple object system using reference counted GC.
- Runtime and compile-time type check with type inference.
- Custom data types.

## Compile
`Tori` is heavily abusing latest c++ features. It requires full support for `c++17`.  

|compiler|version|  
|:--:|:--:|
| `g++` | 7.3 |  
| `clang++` | 6.0.1 |  
| `msvc`    | Visual Studio 2017 15.8 Preview 3 |  

## Binary compatibility
Since this library is heavily relying on inheritance and other features which C++ standard does not specify binary compatibility,
This library currently does NOT (and WONT) support "C++ standard compliant" binary compatibility across compilers.
But we DO want to make this library somewhat binary compatible across some compilers(gcc,clang,msvc) on some platform(Ubuntu,Windows).

## Exsamples

### HelloWorld example
```cpp
#include <tori/core.hpp>
#include <iostream>
// just for example...
using namespace tori;

// String -> Unit
struct Print : Function<Print, String, Unit> {
  ReturnType code() {
    std::cout << eval_arg<0>()->c_str() << std::endl;
    // return type is checked in compile time
    return new Unit();
  }
};

int main() {
  // create closure
  auto print = make_object<Print>();
  // apply
  auto apply = print >> new String("Hello, World!");
  // runtime type check
  check_type<Unit>(apply);
  // eval
  eval(apply); // Hello, World!
}
```

### Simple function
```cpp
// Int -> Int -> Int
struct Add :Function<Add, Int, Int, Int> {
  ReturnType code() {
    auto i1 = *eval_arg<0>();
    auto i2 = *eval_arg<1>();
    return new Int(i1 + i2);
  }
};
```

### Higher order function
```cpp
// (Int -> Int) -> Int -> Int
struct ApplyTwice : Function<ApplyTwice, closure<Int, Int>, Int, Int> {
  ReturnType code() const {
    // arg0 (arg0 arg1)
    return arg<0>() << eval(arg<0>() << arg<1>());
  }
};
```

### Polymorphic function
```cpp
// Bool -> X -> X -> X
struct If : Function<If, Bool, forall<class X>, forall<class X>, forall<class X>> {
  ReturnType code() {
    if (*eval_arg<0>())
      return arg<1>();
    else
      return arg<2>();
  }
};
```

## Basic usage
### Include
```cpp
// core features
#include <tori/core.hpp>

// basic types and utilities
#include <tori/lib.hpp>
```
### Creating object
You can create object with `make_object<T>()`
```cpp
auto i = make_object<Int>(42);
```
Or initialize `object_ptr` from raw pointer.
```cpp
auto i = object_ptr(new Int(42));
```
`object_ptr` is smart pointer like `std::shared_ptr` to manage `Tori` objects.  

You can access to the object just like a pointer.
```cpp
std::cout << *i << std::endl; // 42
```

---

You can define your own data types.  
`BoxedHeapObject` is utility class template to create value type object.  
```cpp
struct MyVector {
  float_t x,y,z;
};
// define custom data type
using MyVectorObject = BoxedHeapObject<MyVector>;

namespace Tori {
  template <>
  struct object_type_traits<MyVectorObject> {
    // define unique name of your type here
    static constexpr const char[] name = "MyVectorObject";
  };
}

// create object
auto myVectorObject = make_object<MyVectorObject>(1.f,2.f,3.f);
```

## Creating closure
`Function` is CRTP utility template to create closure objects.  
You need to specify type of closure in template argument.  
```cpp
// Int -> Int
struct MyFunction :Function<MyFunction, Int, Int> {};
```
In your closure class, you need to define `code()` to write actual code in the closure.  
Use `ReturnType` as return type of `code()`.
```cpp
// Int -> Int
struct MyFunction :Function<MyFunction, Int, Int> {
  ReturnType code() {
    // write your code
  }
};
```
You can write any C++ code in `code()`, but return type will be checked in compile time.  
```cpp
// Int -> Int
struct MyFunction :Function<MyFunction, Int, Int> {
  ReturnType code() {
    return new Int();   // OK!
    return new Double() // Error!
  }
};
```
You can access to argument with `arg<N>()` or `eval_arg<N>()`. Template parameter `N` is index of argument.  
`arg<N>()` returns argument thunk.  
`eval_arg<N>()` evaluates argument first then return the result.
```cpp
// Int -> Int
struct MyFunction :Function<MyFunction, Int, Int> {
  ReturnType code(){
    return arg<0>();      // Thunk<Int> OK!
    return eval_arg<0>(); // Int        OK!
    return arg<1>()       //            Error!
  }
};
```

You can also create higher order functions.  
`operator<<` maps `T1 << T2` to `Apply<T1, T2>`
```cpp
// (Int -> Int) -> Int -> Int
struct MyFunction :Function<MyFunction, closure<Int, Int>, Int, Int> {
  ReturnType code(){
    return arg<0>() << arg<1>();     // Apply<Thunk<closure<Int, Int>>, Thunk<Int>> OK!
    return arg<0>() << new Double(); // Apply<Thunk<closure<Int, Int>>, Double>     Error!
  }
};
```