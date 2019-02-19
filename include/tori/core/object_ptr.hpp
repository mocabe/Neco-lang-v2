// Copyright (c) 2018 mocabe(https://github.com/mocabe)
// This code is licensed under MIT license.

#pragma once

#include "../config/config.hpp"
#include "object_ptr_fwd.hpp"
#include "object_info_table.hpp"

namespace TORI_NS::detail {

  namespace interface {

    /// Destructor
    /// \effects Destroy object with vtable function when reference count become
    /// 0
    template <class T>
    object_ptr<T>::~object_ptr() noexcept
    {
      assert(m_storage.is_pointer());
      if (likely(m_storage.ptr() && !is_static())) {
        // delete object if needed
        if (head()->refcount.fetch_sub() == 1) {
          std::atomic_thread_fence(std::memory_order_acquire);
          info_table()->destroy(m_storage.ptr());
        }
      }
    }

  } // namespace interface

} // namespace TORI_NS::detail