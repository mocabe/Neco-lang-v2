// Copyright (c) 2018-2019 mocabe(https://github.com/mocabe)
// This code is licensed under MIT license.

/// \file
/// SIMD detection macros

#pragma once

#if defined(_MSC_VER)
#  include <intrin.h>
#else
#  include <x86intrin.h>
#endif

namespace TORI_NS::detail {

/// is MMX enabled?
#if defined(__MMX__)
  constexpr bool has_MMX = true;
#else
  constexpr bool has_MMX = false;
#endif

/// is AES enabled?
#if defined(__AES__)
  constexpr bool has_AES = true;
#else
  constexpr bool has_AES = false;
#endif

/// is FMA enabled?
#if defined(__FMA__)
  constexpr bool has_FMA = true;
#else
  constexpr bool has_FMA = false;
#endif

/// is SSE enabled?
#if defined(__SSE__)
  constexpr bool has_SSE = true;
#else
  constexpr bool has_SSE = false;
#endif

/// is SSE2 enabled?
#if defined(__SSE2__)
  constexpr bool has_SSE2 = true;
#else
  constexpr bool has_SSE2 = false;
#endif

/// is SSE3 enabled?
#if defined(__SSE3__)
  constexpr bool has_SSE3 = true;
#else
  constexpr bool has_SSE3 = false;
#endif

/// is SSSE3 enabled?
#if defined(__SSSE3__)
  constexpr bool has_SSSE3 = true;
#else
  constexpr bool has_SSSE3 = false;
#endif

/// is SSE4.1 enabled?
#if defined(__SSE4_1__)
  constexpr bool has_SSE4_1 = true;
#else
  constexpr bool has_SSE4_1 = false;
#endif

/// is SSE4.2 enabled?
#if defined(__SSE4_2__)
  constexpr bool has_SSE4_2 = true;
#else
  constexpr bool has_SSE4_2 = false;
#endif

/// is AVX enabled?
#if defined(__AVX__)
  constexpr bool has_AVX = true;
#else
  constexpr bool has_AVX = false;
#endif

/// is AVX2 enabled?
#if defined(__AVX2__)
  constexpr bool has_AVX2 = true;
#else
  constexpr bool has_AVX2 = false;
#endif
} // namespace TORI_NS::detail