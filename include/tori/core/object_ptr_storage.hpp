// Copyright (c) 2018 mocabe(https://github.com/mocabe)
// This code is licensed under MIT license.

#pragma once

#include "../config/config.hpp"
#include "object_ptr_storage_fwd.hpp"
#include "object_ptr.hpp"

namespace TORI_NS::detail {

  template <class U>
  bool object_ptr_storage::has_pointer_type() const noexcept
  {
    if (!is_pointer())
      return false;

    return ptr() && same_type(ptr()->info_table->obj_type, object_type<U>());
  }

  auto object_ptr_storage::get_immediate_type() const noexcept
  {
    assert(is_immediate());

    if (check_immediate_type(immediate_type_tags::u8))
      return object_type<uint8_t>();
    if (check_immediate_type(immediate_type_tags::u16))
      return object_type<uint16_t>();
    if (check_immediate_type(immediate_type_tags::u32))
      return object_type<uint32_t>();
    if (check_immediate_type(immediate_type_tags::i8))
      return object_type<int8_t>();
    if (check_immediate_type(immediate_type_tags::i16))
      return object_type<int16_t>();
    if (check_immediate_type(immediate_type_tags::i32))
      return object_type<int32_t>();
    if (check_immediate_type(immediate_type_tags::f32))
      return object_type<float>();

    unreachable();
  }

  auto object_ptr_storage::get_pointer_type() const noexcept
  {
    assert(is_pointer());
    assert(ptr());

    auto p = ptr();
    // get type
    return p->info_table->obj_type;
  }

} // namespace TORI_NS::detail