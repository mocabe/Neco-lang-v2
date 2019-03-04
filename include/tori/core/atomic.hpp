// Copyright (c) 2018-2019 mocabe(https://github.com/mocabe)
// This code is licensed under MIT license.

#pragma once

#include "../config/config.hpp"

#include <atomic>
#include <mutex>

namespace TORI_NS::detail {

  /// atomic reference count
  template <class T>
  class atomic_refcount
  {
  public:
    constexpr atomic_refcount() noexcept
      : atomic {0}
    {
    }

    constexpr atomic_refcount(T v) noexcept
      : atomic {v}
    {
    }

    atomic_refcount& operator=(const atomic_refcount& other) noexcept
    {
      store(other.load());
      return *this;
    }

    atomic_refcount& operator=(T v) noexcept
    {
      store(v);
      return *this;
    }

    T load() const noexcept
    {
      return atomic.load(std::memory_order_acquire);
    }

    void store(T v) noexcept
    {
      atomic.store(v, std::memory_order_release);
    }

    /// use memory_order_relaxed
    T fetch_add() noexcept
    {
      return atomic.fetch_add(1u, std::memory_order_relaxed);
    }

    /// use memory_order_release
    T fetch_sub() noexcept
    {
      return atomic.fetch_sub(1u, std::memory_order_release);
    }

  private:
    std::atomic<T> atomic;
    static_assert(std::atomic<T>::is_always_lock_free);
    static_assert(sizeof(T) == sizeof(std::atomic<T>));
  };

  /// atomic spin lock
  template <class T>
  class atomic_spinlock
  {
  public:
    constexpr atomic_spinlock() noexcept
      : m_atomic {0u}
    {
    }

    constexpr atomic_spinlock(bool flg) noexcept
      : m_atomic {flg ? 1u : 0u}
    {
    }

    void lock() noexcept
    {
      while (test_and_set()) {
        /* spin lock */
      }
    }

    void unlock() noexcept
    {
      m_atomic.store(0u, std::memory_order_release);
    }

  private:
    bool test_and_set() noexcept
    {
      return m_atomic.exchange(1u, std::memory_order_acquire);
    }

  private:
    std::atomic<T> m_atomic;
    static_assert(std::atomic<T>::is_always_lock_free);
    static_assert(sizeof(T) == sizeof(std::atomic<T>));
  };

} // namespace TORI_NS::detail