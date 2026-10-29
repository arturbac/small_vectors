// SPDX-FileCopyrightText: 2024 Artur Bać
// SPDX-License-Identifier: MIT
// SPDX-PackageHomePage: https://github.com/arturbac/small_vectors

#pragma once

#define SMALL_VECTORS_VERSION "3.4.0"

#ifdef __clang__
#define small_vectors_clang_do_pragma(x) _Pragma(#x)
#define small_vectors_clang_unsafe_buffer_usage_begin small_vectors_clang_do_pragma(clang unsafe_buffer_usage begin)
#define small_vectors_clang_unsafe_buffer_usage_end small_vectors_clang_do_pragma(clang unsafe_buffer_usage end)
#else
#define small_vectors_clang_unsafe_buffer_usage_begin
#define small_vectors_clang_unsafe_buffer_usage_end
#endif
