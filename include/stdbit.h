/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * stdbit.h
 *
 * Created on: Oct 18, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#ifndef _STDBIT_H
#define _STDBIT_H

#include <__cdefs.h>

__LIBC_BEGIN_DECLS

/* C23 7.18 */
#define __STDC_VERSION_STDBIT_H__ 202311L

/* C23 7.18.1p1: also the exact-width and minimum-width integer type names (repeating a typedef is allowed) */
typedef __SIZE_TYPE__ size_t;
typedef __INT8_TYPE__ int8_t;
typedef __UINT8_TYPE__ uint8_t;
typedef __INT16_TYPE__ int16_t;
typedef __UINT16_TYPE__ uint16_t;
typedef __INT32_TYPE__ int32_t;
typedef __UINT32_TYPE__ uint32_t;
typedef __INT64_TYPE__ int64_t;
typedef __UINT64_TYPE__ uint64_t;
typedef __INT_LEAST8_TYPE__ int_least8_t;
typedef __UINT_LEAST8_TYPE__ uint_least8_t;
typedef __INT_LEAST16_TYPE__ int_least16_t;
typedef __UINT_LEAST16_TYPE__ uint_least16_t;
typedef __INT_LEAST32_TYPE__ int_least32_t;
typedef __UINT_LEAST32_TYPE__ uint_least32_t;
typedef __INT_LEAST64_TYPE__ int_least64_t;
typedef __UINT_LEAST64_TYPE__ uint_least64_t;

/* C23 7.18.2: the compiler knows the byte order, so no conditional is needed */
#define __STDC_ENDIAN_LITTLE__ __ORDER_LITTLE_ENDIAN__
#define __STDC_ENDIAN_BIG__ __ORDER_BIG_ENDIAN__
#define __STDC_ENDIAN_NATIVE__ __BYTE_ORDER__

/* C23 7.18.3 */
unsigned int stdc_leading_zeros_uc(unsigned char value) [[unsequenced]];
unsigned int stdc_leading_zeros_us(unsigned short value) [[unsequenced]];
unsigned int stdc_leading_zeros_ui(unsigned int value) [[unsequenced]];
unsigned int stdc_leading_zeros_ul(unsigned long int value) [[unsequenced]];
unsigned int stdc_leading_zeros_ull(unsigned long long int value) [[unsequenced]];

/* C23 7.18.4 */
unsigned int stdc_leading_ones_uc(unsigned char value) [[unsequenced]];
unsigned int stdc_leading_ones_us(unsigned short value) [[unsequenced]];
unsigned int stdc_leading_ones_ui(unsigned int value) [[unsequenced]];
unsigned int stdc_leading_ones_ul(unsigned long int value) [[unsequenced]];
unsigned int stdc_leading_ones_ull(unsigned long long int value) [[unsequenced]];

/* C23 7.18.5 */
unsigned int stdc_trailing_zeros_uc(unsigned char value) [[unsequenced]];
unsigned int stdc_trailing_zeros_us(unsigned short value) [[unsequenced]];
unsigned int stdc_trailing_zeros_ui(unsigned int value) [[unsequenced]];
unsigned int stdc_trailing_zeros_ul(unsigned long int value) [[unsequenced]];
unsigned int stdc_trailing_zeros_ull(unsigned long long int value) [[unsequenced]];

/* C23 7.18.6 */
unsigned int stdc_trailing_ones_uc(unsigned char value) [[unsequenced]];
unsigned int stdc_trailing_ones_us(unsigned short value) [[unsequenced]];
unsigned int stdc_trailing_ones_ui(unsigned int value) [[unsequenced]];
unsigned int stdc_trailing_ones_ul(unsigned long int value) [[unsequenced]];
unsigned int stdc_trailing_ones_ull(unsigned long long int value) [[unsequenced]];

/* C23 7.18.7 */
unsigned int stdc_first_leading_zero_uc(unsigned char value) [[unsequenced]];
unsigned int stdc_first_leading_zero_us(unsigned short value) [[unsequenced]];
unsigned int stdc_first_leading_zero_ui(unsigned int value) [[unsequenced]];
unsigned int stdc_first_leading_zero_ul(unsigned long int value) [[unsequenced]];
unsigned int stdc_first_leading_zero_ull(unsigned long long int value) [[unsequenced]];

/* C23 7.18.8 */
unsigned int stdc_first_leading_one_uc(unsigned char value) [[unsequenced]];
unsigned int stdc_first_leading_one_us(unsigned short value) [[unsequenced]];
unsigned int stdc_first_leading_one_ui(unsigned int value) [[unsequenced]];
unsigned int stdc_first_leading_one_ul(unsigned long int value) [[unsequenced]];
unsigned int stdc_first_leading_one_ull(unsigned long long int value) [[unsequenced]];

/* C23 7.18.9 */
unsigned int stdc_first_trailing_zero_uc(unsigned char value) [[unsequenced]];
unsigned int stdc_first_trailing_zero_us(unsigned short value) [[unsequenced]];
unsigned int stdc_first_trailing_zero_ui(unsigned int value) [[unsequenced]];
unsigned int stdc_first_trailing_zero_ul(unsigned long int value) [[unsequenced]];
unsigned int stdc_first_trailing_zero_ull(unsigned long long int value) [[unsequenced]];

/* C23 7.18.10 */
unsigned int stdc_first_trailing_one_uc(unsigned char value) [[unsequenced]];
unsigned int stdc_first_trailing_one_us(unsigned short value) [[unsequenced]];
unsigned int stdc_first_trailing_one_ui(unsigned int value) [[unsequenced]];
unsigned int stdc_first_trailing_one_ul(unsigned long int value) [[unsequenced]];
unsigned int stdc_first_trailing_one_ull(unsigned long long int value) [[unsequenced]];

/* C23 7.18.11 */
unsigned int stdc_count_zeros_uc(unsigned char value) [[unsequenced]];
unsigned int stdc_count_zeros_us(unsigned short value) [[unsequenced]];
unsigned int stdc_count_zeros_ui(unsigned int value) [[unsequenced]];
unsigned int stdc_count_zeros_ul(unsigned long int value) [[unsequenced]];
unsigned int stdc_count_zeros_ull(unsigned long long int value) [[unsequenced]];

/* C23 7.18.12 */
unsigned int stdc_count_ones_uc(unsigned char value) [[unsequenced]];
unsigned int stdc_count_ones_us(unsigned short value) [[unsequenced]];
unsigned int stdc_count_ones_ui(unsigned int value) [[unsequenced]];
unsigned int stdc_count_ones_ul(unsigned long int value) [[unsequenced]];
unsigned int stdc_count_ones_ull(unsigned long long int value) [[unsequenced]];

/* C23 7.18.13 */
bool stdc_has_single_bit_uc(unsigned char value) [[unsequenced]];
bool stdc_has_single_bit_us(unsigned short value) [[unsequenced]];
bool stdc_has_single_bit_ui(unsigned int value) [[unsequenced]];
bool stdc_has_single_bit_ul(unsigned long int value) [[unsequenced]];
bool stdc_has_single_bit_ull(unsigned long long int value) [[unsequenced]];

/* C23 7.18.14 */
unsigned int stdc_bit_width_uc(unsigned char value) [[unsequenced]];
unsigned int stdc_bit_width_us(unsigned short value) [[unsequenced]];
unsigned int stdc_bit_width_ui(unsigned int value) [[unsequenced]];
unsigned int stdc_bit_width_ul(unsigned long int value) [[unsequenced]];
unsigned int stdc_bit_width_ull(unsigned long long int value) [[unsequenced]];

/* C23 7.18.15 */
unsigned char stdc_bit_floor_uc(unsigned char value) [[unsequenced]];
unsigned short stdc_bit_floor_us(unsigned short value) [[unsequenced]];
unsigned int stdc_bit_floor_ui(unsigned int value) [[unsequenced]];
unsigned long int stdc_bit_floor_ul(unsigned long int value) [[unsequenced]];
unsigned long long int stdc_bit_floor_ull(unsigned long long int value) [[unsequenced]];

/* C23 7.18.16 */
unsigned char stdc_bit_ceil_uc(unsigned char value) [[unsequenced]];
unsigned short stdc_bit_ceil_us(unsigned short value) [[unsequenced]];
unsigned int stdc_bit_ceil_ui(unsigned int value) [[unsequenced]];
unsigned long int stdc_bit_ceil_ul(unsigned long int value) [[unsequenced]];
unsigned long long int stdc_bit_ceil_ull(unsigned long long int value) [[unsequenced]];

/*
 * The type-generic functions (C23 7.18.1p5 lets them be macros). GCC's
 * __builtin_stdc_* accept every unsigned integer type except bool, including
 * bit-precise ones, and fold to constants for constant arguments. They exist
 * in C only: C++ code calls the typed functions above, or C++20 <bit>.
 */
#ifndef __cplusplus
#define stdc_leading_zeros(value) __builtin_stdc_leading_zeros(value)
#define stdc_leading_ones(value) __builtin_stdc_leading_ones(value)
#define stdc_trailing_zeros(value) __builtin_stdc_trailing_zeros(value)
#define stdc_trailing_ones(value) __builtin_stdc_trailing_ones(value)
#define stdc_first_leading_zero(value) __builtin_stdc_first_leading_zero(value)
#define stdc_first_leading_one(value) __builtin_stdc_first_leading_one(value)
#define stdc_first_trailing_zero(value) __builtin_stdc_first_trailing_zero(value)
#define stdc_first_trailing_one(value) __builtin_stdc_first_trailing_one(value)
#define stdc_count_zeros(value) __builtin_stdc_count_zeros(value)
#define stdc_count_ones(value) __builtin_stdc_count_ones(value)
#define stdc_has_single_bit(value) __builtin_stdc_has_single_bit(value)
#define stdc_bit_width(value) __builtin_stdc_bit_width(value)
#define stdc_bit_floor(value) __builtin_stdc_bit_floor(value)
#define stdc_bit_ceil(value) __builtin_stdc_bit_ceil(value)
#endif

__LIBC_END_DECLS

#endif /* _STDBIT_H */
