/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * stdatomic.h
 *
 * Created on: Oct 19, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#ifndef _STDATOMIC_H
#define _STDATOMIC_H

/* C++ has <atomic>; libstdc++'s C++23 <stdatomic.h> precedes this one on a C++ include path */
#ifndef __cplusplus
/*
 * C23 7.17, built on the compiler's __atomic builtins (PLAN D8). Operations
 * the compiler cannot inline (every read-modify-write on armv6-m, 8-byte
 * operations on 32-bit Arm, odd sizes everywhere) become calls to the
 * __atomic_* library functions libc provides in libc/atomic/
 * (docs/research/threads.md). The generic functions are macros (7.17.1p7).
 */
#define __STDC_VERSION_STDATOMIC_H__ 202311L

/* 7.17.1p4; char8_t is unsigned char (7.30) */
#define ATOMIC_BOOL_LOCK_FREE __GCC_ATOMIC_BOOL_LOCK_FREE
#define ATOMIC_CHAR_LOCK_FREE __GCC_ATOMIC_CHAR_LOCK_FREE
#define ATOMIC_CHAR8_T_LOCK_FREE __GCC_ATOMIC_CHAR_LOCK_FREE
#define ATOMIC_CHAR16_T_LOCK_FREE __GCC_ATOMIC_CHAR16_T_LOCK_FREE
#define ATOMIC_CHAR32_T_LOCK_FREE __GCC_ATOMIC_CHAR32_T_LOCK_FREE
#define ATOMIC_WCHAR_T_LOCK_FREE __GCC_ATOMIC_WCHAR_T_LOCK_FREE
#define ATOMIC_SHORT_LOCK_FREE __GCC_ATOMIC_SHORT_LOCK_FREE
#define ATOMIC_INT_LOCK_FREE __GCC_ATOMIC_INT_LOCK_FREE
#define ATOMIC_LONG_LOCK_FREE __GCC_ATOMIC_LONG_LOCK_FREE
#define ATOMIC_LLONG_LOCK_FREE __GCC_ATOMIC_LLONG_LOCK_FREE
#define ATOMIC_POINTER_LOCK_FREE __GCC_ATOMIC_POINTER_LOCK_FREE

typedef enum {
	memory_order_relaxed = __ATOMIC_RELAXED,
	memory_order_consume = __ATOMIC_CONSUME,
	memory_order_acquire = __ATOMIC_ACQUIRE,
	memory_order_release = __ATOMIC_RELEASE,
	memory_order_acq_rel = __ATOMIC_ACQ_REL,
	memory_order_seq_cst = __ATOMIC_SEQ_CST
} memory_order;

/* 7.17.8: the flag is a bool that test-and-set operates on */
typedef struct {
	bool __value;
} atomic_flag;

#define ATOMIC_FLAG_INIT \
	{                    \
		false            \
	}

/* 7.17.6 */
typedef _Atomic bool atomic_bool;
typedef _Atomic char atomic_char;
typedef _Atomic signed char atomic_schar;
typedef _Atomic unsigned char atomic_uchar;
typedef _Atomic short atomic_short;
typedef _Atomic unsigned short atomic_ushort;
typedef _Atomic int atomic_int;
typedef _Atomic unsigned int atomic_uint;
typedef _Atomic long atomic_long;
typedef _Atomic unsigned long atomic_ulong;
typedef _Atomic long long atomic_llong;
typedef _Atomic unsigned long long atomic_ullong;
typedef _Atomic unsigned char atomic_char8_t;
typedef _Atomic __CHAR16_TYPE__ atomic_char16_t;
typedef _Atomic __CHAR32_TYPE__ atomic_char32_t;
typedef _Atomic __WCHAR_TYPE__ atomic_wchar_t;
typedef _Atomic __INT_LEAST8_TYPE__ atomic_int_least8_t;
typedef _Atomic __UINT_LEAST8_TYPE__ atomic_uint_least8_t;
typedef _Atomic __INT_LEAST16_TYPE__ atomic_int_least16_t;
typedef _Atomic __UINT_LEAST16_TYPE__ atomic_uint_least16_t;
typedef _Atomic __INT_LEAST32_TYPE__ atomic_int_least32_t;
typedef _Atomic __UINT_LEAST32_TYPE__ atomic_uint_least32_t;
typedef _Atomic __INT_LEAST64_TYPE__ atomic_int_least64_t;
typedef _Atomic __UINT_LEAST64_TYPE__ atomic_uint_least64_t;
typedef _Atomic __INT_FAST8_TYPE__ atomic_int_fast8_t;
typedef _Atomic __UINT_FAST8_TYPE__ atomic_uint_fast8_t;
typedef _Atomic __INT_FAST16_TYPE__ atomic_int_fast16_t;
typedef _Atomic __UINT_FAST16_TYPE__ atomic_uint_fast16_t;
typedef _Atomic __INT_FAST32_TYPE__ atomic_int_fast32_t;
typedef _Atomic __UINT_FAST32_TYPE__ atomic_uint_fast32_t;
typedef _Atomic __INT_FAST64_TYPE__ atomic_int_fast64_t;
typedef _Atomic __UINT_FAST64_TYPE__ atomic_uint_fast64_t;
typedef _Atomic __INTPTR_TYPE__ atomic_intptr_t;
typedef _Atomic __UINTPTR_TYPE__ atomic_uintptr_t;
typedef _Atomic __SIZE_TYPE__ atomic_size_t;
typedef _Atomic __PTRDIFF_TYPE__ atomic_ptrdiff_t;
typedef _Atomic __INTMAX_TYPE__ atomic_intmax_t;
typedef _Atomic __UINTMAX_TYPE__ atomic_uintmax_t;

/* 7.17.2.1: initialisation need not be atomic */
#define atomic_init(obj, value) __extension__({ typeof_unqual(*(obj)) __libc_init = (value); __atomic_store((obj), &__libc_init, __ATOMIC_RELAXED); })

/* 7.17.3.1 */
#define kill_dependency(y) (y)

/* 7.17.4 */
void atomic_thread_fence(memory_order order);
void atomic_signal_fence(memory_order order);

#define atomic_thread_fence(order) __atomic_thread_fence(order)
#define atomic_signal_fence(order) __atomic_signal_fence(order)

/* 7.17.5.1 */
#define atomic_is_lock_free(obj) __atomic_is_lock_free(sizeof(*(obj)), (obj))

/* 7.17.7.1-7.17.7.4: the generic builtins accept any atomic type */
#define atomic_store_explicit(object, desired, order) __extension__({ typeof_unqual(*(object)) __libc_desired = (desired); __atomic_store((object), &__libc_desired, (order)); })
#define atomic_store(object, desired) atomic_store_explicit(object, desired, memory_order_seq_cst)

#define atomic_load_explicit(object, order) __extension__({ typeof_unqual(*(object)) __libc_result; __atomic_load((object), &__libc_result, (order)); __libc_result; })
#define atomic_load(object) atomic_load_explicit(object, memory_order_seq_cst)

#define atomic_exchange_explicit(object, desired, order) __extension__({ typeof_unqual(*(object)) __libc_desired = (desired); typeof_unqual(*(object)) __libc_result; __atomic_exchange((object), &__libc_desired, &__libc_result, (order)); __libc_result; })
#define atomic_exchange(object, desired) atomic_exchange_explicit(object, desired, memory_order_seq_cst)

#define atomic_compare_exchange_strong_explicit(object, expected, desired, success, failure) __extension__({ typeof_unqual(*(object)) __libc_desired = (desired); __atomic_compare_exchange((object), (expected), &__libc_desired, false, (success), (failure)); })
#define atomic_compare_exchange_strong(object, expected, desired) atomic_compare_exchange_strong_explicit(object, expected, desired, memory_order_seq_cst, memory_order_seq_cst)
#define atomic_compare_exchange_weak_explicit(object, expected, desired, success, failure) __extension__({ typeof_unqual(*(object)) __libc_desired = (desired); __atomic_compare_exchange((object), (expected), &__libc_desired, true, (success), (failure)); })
#define atomic_compare_exchange_weak(object, expected, desired) atomic_compare_exchange_weak_explicit(object, expected, desired, memory_order_seq_cst, memory_order_seq_cst)

/*
 * 7.17.7.5. The builtins add bytes to pointers; C adds elements, so the
 * operand is scaled by the distance from 0 to 0 + 1 in the object's type,
 * which is 1 for integers and the element size for pointers.
 */
#define __libc_atomic_scale(object) ((__INTPTR_TYPE__)((typeof_unqual(*(object)))0 + 1))

#define atomic_fetch_add_explicit(object, operand, order) __atomic_fetch_add((object), (operand) * __libc_atomic_scale(object), (order))
#define atomic_fetch_add(object, operand) atomic_fetch_add_explicit(object, operand, memory_order_seq_cst)
#define atomic_fetch_sub_explicit(object, operand, order) __atomic_fetch_sub((object), (operand) * __libc_atomic_scale(object), (order))
#define atomic_fetch_sub(object, operand) atomic_fetch_sub_explicit(object, operand, memory_order_seq_cst)
#define atomic_fetch_or_explicit(object, operand, order) __atomic_fetch_or((object), (operand), (order))
#define atomic_fetch_or(object, operand) atomic_fetch_or_explicit(object, operand, memory_order_seq_cst)
#define atomic_fetch_xor_explicit(object, operand, order) __atomic_fetch_xor((object), (operand), (order))
#define atomic_fetch_xor(object, operand) atomic_fetch_xor_explicit(object, operand, memory_order_seq_cst)
#define atomic_fetch_and_explicit(object, operand, order) __atomic_fetch_and((object), (operand), (order))
#define atomic_fetch_and(object, operand) atomic_fetch_and_explicit(object, operand, memory_order_seq_cst)

/* 7.17.8.1, 7.17.8.2: functions, and macros for them */
bool atomic_flag_test_and_set(volatile atomic_flag *object);
bool atomic_flag_test_and_set_explicit(volatile atomic_flag *object, memory_order order);
void atomic_flag_clear(volatile atomic_flag *object);
void atomic_flag_clear_explicit(volatile atomic_flag *object, memory_order order);

#define atomic_flag_test_and_set_explicit(object, order) __atomic_test_and_set(&(object)->__value, (order))
#define atomic_flag_test_and_set(object) atomic_flag_test_and_set_explicit(object, memory_order_seq_cst)
#define atomic_flag_clear_explicit(object, order) __atomic_clear(&(object)->__value, (order))
#define atomic_flag_clear(object) atomic_flag_clear_explicit(object, memory_order_seq_cst)

#endif /* __cplusplus */

#endif /* _STDATOMIC_H */
