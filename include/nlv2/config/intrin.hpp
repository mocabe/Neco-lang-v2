// Copyright (c) 2018 mocabe(https://github.com/mocabe)
// This code is licensed under MIT license.

/// \file
/// SIMD detection macros

#pragma once

#if defined(_MSC_VER)
#  include <intrin.h>
#else
#  include <x86intrin.h>
#endif

namespace Neco::lang::v2 {

#if defined(__SSE__)
  constexpr bool has_SSE = true;
#else
  constexpr bool has_SSE = false;
#endif

#if defined(__SSE2__)
  constexpr bool has_SSE2 = true;
#else
  constexpr bool has_SSE2 = false;
#endif
#if defined(__SSE3__)
  constexpr bool has_SSE3 = true;
#else
  constexpr bool has_SSE3 = false;
#endif
#if defined(__SSE4_1__)
  constexpr bool has_SSE4_1 = true;
#else
  constexpr bool has_SSE4_1 = false;
#endif
#if defined(__SSE4_2__)
  constexpr bool has_SSE4_2 = true;
#else
  constexpr bool has_SSE4_2 = false;
#endif
#if defined(__AVX__)
  constexpr bool has_AVX = true;
#else
  constexpr bool has_AVX = false;
#endif
#if defined(__AVX2__)
  constexpr bool has_AVX2 = true;
#else
  constexpr bool has_AVX2 = false;
#endif

} // namespace Neco::lang::v2