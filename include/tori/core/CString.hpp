#pragma once

#include "DynamicTypeUtil.hpp"
#include <string>
#include <cstring>
#include <string>

namespace TORI_NS::detail {
  class CStringValue {
  public:
    CStringValue(const std::string& str) {
      m_ptr = new char[str.size() + 1];
      std::copy(str.c_str(), str.c_str() + str.size() + 1, m_ptr);
    }
    CStringValue(const CStringValue& other) {
      auto len = std::strlen(other.m_ptr);
      m_ptr = new char[len + 1];
      std::copy(other.m_ptr, other.m_ptr + len + 1, m_ptr);
    }
    CStringValue(CStringValue&& other) {
      m_ptr = other.m_ptr;
      other.m_ptr = nullptr;
    }
    ~CStringValue() noexcept {
      delete[] m_ptr;
    }
    const char* c_str() const noexcept {
      return m_ptr;
    }

  private:
    char* m_ptr; // TODO use char8_t
  };

  namespace interface {
    /// CString
    using CString = BoxedHeapObject<CStringValue>;
  } // namespace interface

} // namespace TORI_NS::detail
