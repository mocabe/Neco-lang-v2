// Copyright (c) 2018 mocabe(https://github.com/mocabe)
// This code is licensed under MIT license.

#pragma once

#include "HeapObject.hpp"
#include <utility>
#include <cassert>
#include <type_traits>
#include <new>

/// \file BoxedHeapObject.hpp

namespace TORI_NS::detail {

  /// tag type to initialize object with 0 reference count
  struct static_construct_t {
    explicit static_construct_t() = default;
  };
  namespace interface {
    /// static_construct
    inline constexpr static_construct_t static_construct = static_construct_t();
    // forward decl
    template <class T>
    struct object_type; // see TypeGen.hpp

  } // namespace interface

  /// \brief vtable function to delete object
  ///
  /// vtable function to delete heap allocated object.
  template <class T>
  void vtbl_destroy_func(HeapObject *obj) noexcept {
    static_assert(
      std::is_nothrow_destructible_v<T>, "should have nothrow destructor");
    auto *p = static_cast<T *>(obj);
    delete p;
  }

  /// \brief vtable function to clone object.
  ///
  /// vtable function to clone heap-allocated object.
  /// \param T value type
  /// \returns pointer to generated object.
  /// \notes return nullptr when allocation/initialization failed.
  template <class T>
  HeapObject *vtbl_clone_func(const HeapObject *obj) noexcept {
    try {
      auto p = static_cast<const T *>(obj);
      return new T{*p};
    } catch (...) { return nullptr; }
  }

  namespace interface {

    /// \brief Heap-allocated object generator.
    /// \param T value type
    /// \param AllocatorTemplate allocator
    template <class T, template <class> class AllocatorTemplate>
    struct BoxedHeapObject : HeapObject {
      /// value type
      using value_type = T;
      /// allocator type
      using allocator_type = AllocatorTemplate<BoxedHeapObject>;
      /// term
      using term = TmValue<BoxedHeapObject>;

      /// info table initializer
      struct info_table_initializer {
        /// static object info table
        static const ObjectInfoTable info_table;
      };

      /// Ctor
      template <
        class U,
        class... Args,
        class = std::enable_if_t<
          !std::is_same_v<std::decay_t<U>, BoxedHeapObject> &&
          !std::is_same_v<std::decay_t<U>, static_construct_t>>>
      constexpr BoxedHeapObject(U &&u, Args &&... args)
        : HeapObject{1u, &info_table_initializer::info_table}
        , value{std::forward<U>(u), std::forward<Args>(args)...} {}

      /// Ctor (static initialization)
      template <class... Args>
      constexpr BoxedHeapObject(static_construct_t, Args &&... args)
        : BoxedHeapObject(std::forward<Args>(args)...) {
        // set refcount ZERO to avoid deletion
        refcount.atomic = 0;
      }

      /// Ctor
      constexpr BoxedHeapObject()
        : HeapObject{1u, &info_table_initializer::info_table}, value{} {}
      /// Copy ctor
      constexpr BoxedHeapObject(const BoxedHeapObject &obj)
        : HeapObject{obj}, value{obj.value} {}
      /// Move ctor
      constexpr BoxedHeapObject(BoxedHeapObject &&obj)
        : HeapObject{obj}, value{std::move(obj.value)} {}

      /// operator=
      BoxedHeapObject &operator=(const BoxedHeapObject &obj) {
        HeapObject::operator=(obj);
        value = obj.value;
      }
      /// operator=
      BoxedHeapObject &operator=(BoxedHeapObject &&obj) {
        HeapObject::operator=(obj);
        value = std::move(obj.value);
      }
      /// operator new
      void *operator new(std::size_t n) {
        AllocatorTemplate<BoxedHeapObject> allocator;
        return allocator.allocate(n);
      }
      /// operator delete
      void operator delete(void *p) noexcept {
        AllocatorTemplate<BoxedHeapObject> allocator;
        allocator.deallocate(static_cast<BoxedHeapObject *>(p), 1);
      }
      /// operator delete
      void operator delete(void *p, std::size_t n) noexcept {
        AllocatorTemplate<BoxedHeapObject> allocator;
        allocator.deallocate(static_cast<BoxedHeapObject *>(p), n);
      }
      /// value
      T value;
    };

    // Initialize object header
    template <class T, template <class> class AllocatorTemplate>
    const ObjectInfoTable BoxedHeapObject<T, AllocatorTemplate>::
      info_table_initializer::info_table = {
        object_type<BoxedHeapObject>::get(), //
        sizeof(BoxedHeapObject),             //
        object_header_extend_bytes,          //
        vtbl_destroy_func<BoxedHeapObject>,  //
        vtbl_clone_func<BoxedHeapObject>};

  } // namespace interface
} // namespace TORI_NS::detail