/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * headers.c
 *
 * Created on: Oct 18, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include <errno.h>
#include <fenv.h>
#include <float.h>
#include <iso646.h>
#include <limits.h>
#include <stdalign.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdnoreturn.h>

/*
 * Freestanding header checks (P1.01). Most checks are at compile time: every
 * <limits.h> and <stdint.h> limit must be usable in #if (C23 5.2.5.3.2,
 * 7.22.2) and agree with the types it describes.
 */

/* D50: Annex F (<stdc-predef.h>) is claimed exactly where <fenv.h> has every rounding direction */
#if defined(FE_UPWARD) != defined(__STDC_IEC_60559_BFP__) || defined(FE_UPWARD) != defined(__STDC_IEC_559__)
#error the Annex F claim disagrees with <fenv.h>
#endif
#if defined(__STDC_IEC_60559_COMPLEX__) || defined(__STDC_IEC_60559_DFP__)
#error Annex G and decimal floating point are not claimed
#endif

/* Each limit must be an integer constant expression usable in #if */
#if !(CHAR_BIT == CHAR_BIT)
#error CHAR_BIT unusable in #if
#endif
#if !(SCHAR_MIN == SCHAR_MIN)
#error SCHAR_MIN unusable in #if
#endif
#if !(SCHAR_MAX == SCHAR_MAX)
#error SCHAR_MAX unusable in #if
#endif
#if !(UCHAR_MAX == UCHAR_MAX)
#error UCHAR_MAX unusable in #if
#endif
#if !(CHAR_MIN == CHAR_MIN)
#error CHAR_MIN unusable in #if
#endif
#if !(CHAR_MAX == CHAR_MAX)
#error CHAR_MAX unusable in #if
#endif
#if !(SHRT_MIN == SHRT_MIN)
#error SHRT_MIN unusable in #if
#endif
#if !(SHRT_MAX == SHRT_MAX)
#error SHRT_MAX unusable in #if
#endif
#if !(USHRT_MAX == USHRT_MAX)
#error USHRT_MAX unusable in #if
#endif
#if !(INT_MIN == INT_MIN)
#error INT_MIN unusable in #if
#endif
#if !(INT_MAX == INT_MAX)
#error INT_MAX unusable in #if
#endif
#if !(UINT_MAX == UINT_MAX)
#error UINT_MAX unusable in #if
#endif
#if !(LONG_MIN == LONG_MIN)
#error LONG_MIN unusable in #if
#endif
#if !(LONG_MAX == LONG_MAX)
#error LONG_MAX unusable in #if
#endif
#if !(ULONG_MAX == ULONG_MAX)
#error ULONG_MAX unusable in #if
#endif
#if !(LLONG_MIN == LLONG_MIN)
#error LLONG_MIN unusable in #if
#endif
#if !(LLONG_MAX == LLONG_MAX)
#error LLONG_MAX unusable in #if
#endif
#if !(ULLONG_MAX == ULLONG_MAX)
#error ULLONG_MAX unusable in #if
#endif
#if !(MB_LEN_MAX == MB_LEN_MAX)
#error MB_LEN_MAX unusable in #if
#endif
#if !(BOOL_WIDTH == BOOL_WIDTH)
#error BOOL_WIDTH unusable in #if
#endif
#if !(CHAR_WIDTH == CHAR_WIDTH)
#error CHAR_WIDTH unusable in #if
#endif
#if !(SCHAR_WIDTH == SCHAR_WIDTH)
#error SCHAR_WIDTH unusable in #if
#endif
#if !(UCHAR_WIDTH == UCHAR_WIDTH)
#error UCHAR_WIDTH unusable in #if
#endif
#if !(SHRT_WIDTH == SHRT_WIDTH)
#error SHRT_WIDTH unusable in #if
#endif
#if !(USHRT_WIDTH == USHRT_WIDTH)
#error USHRT_WIDTH unusable in #if
#endif
#if !(INT_WIDTH == INT_WIDTH)
#error INT_WIDTH unusable in #if
#endif
#if !(UINT_WIDTH == UINT_WIDTH)
#error UINT_WIDTH unusable in #if
#endif
#if !(LONG_WIDTH == LONG_WIDTH)
#error LONG_WIDTH unusable in #if
#endif
#if !(ULONG_WIDTH == ULONG_WIDTH)
#error ULONG_WIDTH unusable in #if
#endif
#if !(LLONG_WIDTH == LLONG_WIDTH)
#error LLONG_WIDTH unusable in #if
#endif
#if !(ULLONG_WIDTH == ULLONG_WIDTH)
#error ULLONG_WIDTH unusable in #if
#endif
/* Only where the compiler has _BitInt (PLAN D127); then it is at least ULLONG_WIDTH (C23 5.2.4.2.1) */
#if defined(__BITINT_MAXWIDTH__) && !(BITINT_MAXWIDTH >= ULLONG_WIDTH)
#error BITINT_MAXWIDTH unusable in #if, or below ULLONG_WIDTH
#endif
#if !defined(__BITINT_MAXWIDTH__) && defined(BITINT_MAXWIDTH)
#error BITINT_MAXWIDTH defined without _BitInt
#endif
#if !(INT8_MIN == INT8_MIN)
#error INT8_MIN unusable in #if
#endif
#if !(INT8_MAX == INT8_MAX)
#error INT8_MAX unusable in #if
#endif
#if !(UINT8_MAX == UINT8_MAX)
#error UINT8_MAX unusable in #if
#endif
#if !(INT8_WIDTH == INT8_WIDTH)
#error INT8_WIDTH unusable in #if
#endif
#if !(UINT8_WIDTH == UINT8_WIDTH)
#error UINT8_WIDTH unusable in #if
#endif
#if !(INT_LEAST8_MIN == INT_LEAST8_MIN)
#error INT_LEAST8_MIN unusable in #if
#endif
#if !(INT_LEAST8_MAX == INT_LEAST8_MAX)
#error INT_LEAST8_MAX unusable in #if
#endif
#if !(UINT_LEAST8_MAX == UINT_LEAST8_MAX)
#error UINT_LEAST8_MAX unusable in #if
#endif
#if !(INT_LEAST8_WIDTH == INT_LEAST8_WIDTH)
#error INT_LEAST8_WIDTH unusable in #if
#endif
#if !(UINT_LEAST8_WIDTH == UINT_LEAST8_WIDTH)
#error UINT_LEAST8_WIDTH unusable in #if
#endif
#if !(INT_FAST8_MIN == INT_FAST8_MIN)
#error INT_FAST8_MIN unusable in #if
#endif
#if !(INT_FAST8_MAX == INT_FAST8_MAX)
#error INT_FAST8_MAX unusable in #if
#endif
#if !(UINT_FAST8_MAX == UINT_FAST8_MAX)
#error UINT_FAST8_MAX unusable in #if
#endif
#if !(INT_FAST8_WIDTH == INT_FAST8_WIDTH)
#error INT_FAST8_WIDTH unusable in #if
#endif
#if !(UINT_FAST8_WIDTH == UINT_FAST8_WIDTH)
#error UINT_FAST8_WIDTH unusable in #if
#endif
#if !(INT16_MIN == INT16_MIN)
#error INT16_MIN unusable in #if
#endif
#if !(INT16_MAX == INT16_MAX)
#error INT16_MAX unusable in #if
#endif
#if !(UINT16_MAX == UINT16_MAX)
#error UINT16_MAX unusable in #if
#endif
#if !(INT16_WIDTH == INT16_WIDTH)
#error INT16_WIDTH unusable in #if
#endif
#if !(UINT16_WIDTH == UINT16_WIDTH)
#error UINT16_WIDTH unusable in #if
#endif
#if !(INT_LEAST16_MIN == INT_LEAST16_MIN)
#error INT_LEAST16_MIN unusable in #if
#endif
#if !(INT_LEAST16_MAX == INT_LEAST16_MAX)
#error INT_LEAST16_MAX unusable in #if
#endif
#if !(UINT_LEAST16_MAX == UINT_LEAST16_MAX)
#error UINT_LEAST16_MAX unusable in #if
#endif
#if !(INT_LEAST16_WIDTH == INT_LEAST16_WIDTH)
#error INT_LEAST16_WIDTH unusable in #if
#endif
#if !(UINT_LEAST16_WIDTH == UINT_LEAST16_WIDTH)
#error UINT_LEAST16_WIDTH unusable in #if
#endif
#if !(INT_FAST16_MIN == INT_FAST16_MIN)
#error INT_FAST16_MIN unusable in #if
#endif
#if !(INT_FAST16_MAX == INT_FAST16_MAX)
#error INT_FAST16_MAX unusable in #if
#endif
#if !(UINT_FAST16_MAX == UINT_FAST16_MAX)
#error UINT_FAST16_MAX unusable in #if
#endif
#if !(INT_FAST16_WIDTH == INT_FAST16_WIDTH)
#error INT_FAST16_WIDTH unusable in #if
#endif
#if !(UINT_FAST16_WIDTH == UINT_FAST16_WIDTH)
#error UINT_FAST16_WIDTH unusable in #if
#endif
#if !(INT32_MIN == INT32_MIN)
#error INT32_MIN unusable in #if
#endif
#if !(INT32_MAX == INT32_MAX)
#error INT32_MAX unusable in #if
#endif
#if !(UINT32_MAX == UINT32_MAX)
#error UINT32_MAX unusable in #if
#endif
#if !(INT32_WIDTH == INT32_WIDTH)
#error INT32_WIDTH unusable in #if
#endif
#if !(UINT32_WIDTH == UINT32_WIDTH)
#error UINT32_WIDTH unusable in #if
#endif
#if !(INT_LEAST32_MIN == INT_LEAST32_MIN)
#error INT_LEAST32_MIN unusable in #if
#endif
#if !(INT_LEAST32_MAX == INT_LEAST32_MAX)
#error INT_LEAST32_MAX unusable in #if
#endif
#if !(UINT_LEAST32_MAX == UINT_LEAST32_MAX)
#error UINT_LEAST32_MAX unusable in #if
#endif
#if !(INT_LEAST32_WIDTH == INT_LEAST32_WIDTH)
#error INT_LEAST32_WIDTH unusable in #if
#endif
#if !(UINT_LEAST32_WIDTH == UINT_LEAST32_WIDTH)
#error UINT_LEAST32_WIDTH unusable in #if
#endif
#if !(INT_FAST32_MIN == INT_FAST32_MIN)
#error INT_FAST32_MIN unusable in #if
#endif
#if !(INT_FAST32_MAX == INT_FAST32_MAX)
#error INT_FAST32_MAX unusable in #if
#endif
#if !(UINT_FAST32_MAX == UINT_FAST32_MAX)
#error UINT_FAST32_MAX unusable in #if
#endif
#if !(INT_FAST32_WIDTH == INT_FAST32_WIDTH)
#error INT_FAST32_WIDTH unusable in #if
#endif
#if !(UINT_FAST32_WIDTH == UINT_FAST32_WIDTH)
#error UINT_FAST32_WIDTH unusable in #if
#endif
#if !(INT64_MIN == INT64_MIN)
#error INT64_MIN unusable in #if
#endif
#if !(INT64_MAX == INT64_MAX)
#error INT64_MAX unusable in #if
#endif
#if !(UINT64_MAX == UINT64_MAX)
#error UINT64_MAX unusable in #if
#endif
#if !(INT64_WIDTH == INT64_WIDTH)
#error INT64_WIDTH unusable in #if
#endif
#if !(UINT64_WIDTH == UINT64_WIDTH)
#error UINT64_WIDTH unusable in #if
#endif
#if !(INT_LEAST64_MIN == INT_LEAST64_MIN)
#error INT_LEAST64_MIN unusable in #if
#endif
#if !(INT_LEAST64_MAX == INT_LEAST64_MAX)
#error INT_LEAST64_MAX unusable in #if
#endif
#if !(UINT_LEAST64_MAX == UINT_LEAST64_MAX)
#error UINT_LEAST64_MAX unusable in #if
#endif
#if !(INT_LEAST64_WIDTH == INT_LEAST64_WIDTH)
#error INT_LEAST64_WIDTH unusable in #if
#endif
#if !(UINT_LEAST64_WIDTH == UINT_LEAST64_WIDTH)
#error UINT_LEAST64_WIDTH unusable in #if
#endif
#if !(INT_FAST64_MIN == INT_FAST64_MIN)
#error INT_FAST64_MIN unusable in #if
#endif
#if !(INT_FAST64_MAX == INT_FAST64_MAX)
#error INT_FAST64_MAX unusable in #if
#endif
#if !(UINT_FAST64_MAX == UINT_FAST64_MAX)
#error UINT_FAST64_MAX unusable in #if
#endif
#if !(INT_FAST64_WIDTH == INT_FAST64_WIDTH)
#error INT_FAST64_WIDTH unusable in #if
#endif
#if !(UINT_FAST64_WIDTH == UINT_FAST64_WIDTH)
#error UINT_FAST64_WIDTH unusable in #if
#endif
#if !(INTPTR_MIN == INTPTR_MIN)
#error INTPTR_MIN unusable in #if
#endif
#if !(INTPTR_MAX == INTPTR_MAX)
#error INTPTR_MAX unusable in #if
#endif
#if !(UINTPTR_MAX == UINTPTR_MAX)
#error UINTPTR_MAX unusable in #if
#endif
#if !(INTPTR_WIDTH == INTPTR_WIDTH)
#error INTPTR_WIDTH unusable in #if
#endif
#if !(UINTPTR_WIDTH == UINTPTR_WIDTH)
#error UINTPTR_WIDTH unusable in #if
#endif
#if !(INTMAX_MIN == INTMAX_MIN)
#error INTMAX_MIN unusable in #if
#endif
#if !(INTMAX_MAX == INTMAX_MAX)
#error INTMAX_MAX unusable in #if
#endif
#if !(UINTMAX_MAX == UINTMAX_MAX)
#error UINTMAX_MAX unusable in #if
#endif
#if !(INTMAX_WIDTH == INTMAX_WIDTH)
#error INTMAX_WIDTH unusable in #if
#endif
#if !(UINTMAX_WIDTH == UINTMAX_WIDTH)
#error UINTMAX_WIDTH unusable in #if
#endif
#if !(PTRDIFF_MIN == PTRDIFF_MIN)
#error PTRDIFF_MIN unusable in #if
#endif
#if !(PTRDIFF_MAX == PTRDIFF_MAX)
#error PTRDIFF_MAX unusable in #if
#endif
#if !(PTRDIFF_WIDTH == PTRDIFF_WIDTH)
#error PTRDIFF_WIDTH unusable in #if
#endif
#if !(SIG_ATOMIC_MIN == SIG_ATOMIC_MIN)
#error SIG_ATOMIC_MIN unusable in #if
#endif
#if !(SIG_ATOMIC_MAX == SIG_ATOMIC_MAX)
#error SIG_ATOMIC_MAX unusable in #if
#endif
#if !(SIG_ATOMIC_WIDTH == SIG_ATOMIC_WIDTH)
#error SIG_ATOMIC_WIDTH unusable in #if
#endif
#if !(SIZE_MAX == SIZE_MAX)
#error SIZE_MAX unusable in #if
#endif
#if !(SIZE_WIDTH == SIZE_WIDTH)
#error SIZE_WIDTH unusable in #if
#endif
#if !(WCHAR_MIN == WCHAR_MIN)
#error WCHAR_MIN unusable in #if
#endif
#if !(WCHAR_MAX == WCHAR_MAX)
#error WCHAR_MAX unusable in #if
#endif
#if !(WCHAR_WIDTH == WCHAR_WIDTH)
#error WCHAR_WIDTH unusable in #if
#endif
#if !(WINT_MIN == WINT_MIN)
#error WINT_MIN unusable in #if
#endif
#if !(WINT_MAX == WINT_MAX)
#error WINT_MAX unusable in #if
#endif
#if !(WINT_WIDTH == WINT_WIDTH)
#error WINT_WIDTH unusable in #if
#endif

static_assert(__STDC_VERSION_STDDEF_H__ == 202311L);
static_assert(__STDC_VERSION_STDINT_H__ == 202311L);
static_assert(__STDC_VERSION_LIMITS_H__ == 202311L);
static_assert(__STDC_VERSION_FLOAT_H__ == 202311L);
static_assert(__STDC_VERSION_STDARG_H__ == 202311L);
static_assert(__bool_true_false_are_defined == 1);

static_assert(EDOM > 0 && ERANGE > 0 && EILSEQ > 0 && EDOM != ERANGE && EDOM != EILSEQ && ERANGE != EILSEQ);

static_assert(sizeof(int8_t) == 1 && sizeof(int16_t) == 2 && sizeof(int32_t) == 4 && sizeof(int64_t) == 8);
static_assert(INT8_MIN == -128 && INT8_MAX == 127 && UINT8_MAX == 255);
static_assert(INT16_MIN == -32768 && INT16_MAX == 32767 && UINT16_MAX == 65535);
static_assert(INT32_MAX == 2147483647 && UINT32_MAX == 4294967295U);
static_assert(INT64_MAX == 9223372036854775807LL && UINT64_MAX == 18446744073709551615ULL);
static_assert(INT32_MIN == -INT32_MAX - 1 && INT64_MIN == -INT64_MAX - 1);
static_assert(sizeof(intptr_t) == sizeof(void *) && sizeof(uintptr_t) == sizeof(void *));
static_assert(sizeof(size_t) * CHAR_BIT == SIZE_WIDTH && sizeof(ptrdiff_t) * CHAR_BIT == PTRDIFF_WIDTH);
static_assert(SIZE_MAX == (size_t)-1);
static_assert(UINTMAX_MAX == (uintmax_t)-1 && INTMAX_MAX == (intmax_t)(UINTMAX_MAX >> 1));
static_assert(INT8_C(-1) == -1 && UINT32_C(1) == 1U && INT64_C(1) << 62 > 0 && UINTMAX_C(1) << 63 > 0);
static_assert(_Generic(INT64_C(0), int64_t: 1, default: 0) || sizeof(INT64_C(0)) >= sizeof(int64_t));

static_assert(CHAR_BIT == 8);
static_assert(SCHAR_MIN == -128 && SCHAR_MAX == 127 && UCHAR_MAX == 255);
static_assert(CHAR_MIN == ((char)-1 < 0 ? SCHAR_MIN : 0) && CHAR_MAX == ((char)-1 < 0 ? SCHAR_MAX : UCHAR_MAX));
static_assert(SHRT_MAX == INT16_MAX && USHRT_MAX == UINT16_MAX);
static_assert(INT_MAX == INT32_MAX && UINT_MAX == UINT32_MAX && INT_MIN == INT32_MIN);
static_assert(LONG_MAX == (long)(ULONG_MAX >> 1) && LONG_MIN == -LONG_MAX - 1);
static_assert(LLONG_MAX == INT64_MAX && ULLONG_MAX == UINT64_MAX);
static_assert(UINT_MAX == (unsigned int)-1 && ULONG_MAX == (unsigned long)-1 && ULLONG_MAX == (unsigned long long)-1);
static_assert(INT_WIDTH == sizeof(int) * CHAR_BIT && LONG_WIDTH == sizeof(long) * CHAR_BIT && BOOL_WIDTH == 1);
static_assert(MB_LEN_MAX >= 1);

static_assert(FLT_RADIX == 2 && FLT_MANT_DIG == 24 && DBL_MANT_DIG == 53);
static_assert(LDBL_MANT_DIG == DBL_MANT_DIG, "long double is binary64 on every target (PLAN D7)");
static_assert(FLT_EPSILON == 0x1p-23f && DBL_EPSILON == 0x1p-52 && DBL_MAX == 0x1.fffffffffffffp1023);
static_assert(FLT_MIN == 0x1p-126f && FLT_TRUE_MIN == 0x1p-149f && DBL_TRUE_MIN == 0x1p-1074);
static_assert(FLT_IS_IEC_60559 == 1 && DBL_IS_IEC_60559 == 1);

struct offsets {
	char c;
	double d;
	int i;
};
static_assert(offsetof(struct offsets, c) == 0 && offsetof(struct offsets, i) == 2 * sizeof(double));
static_assert(alignof(max_align_t) >= alignof(long long) && alignof(max_align_t) >= alignof(long double));
static_assert(sizeof(nullptr_t) == sizeof(void *));
static_assert(sizeof(wchar_t) * CHAR_BIT == WCHAR_WIDTH);

static int failures;

static void check(bool ok, const char *what)
{
	fputs(ok ? "pass: " : "fail: ", stdout);
	puts(what);
	if (!ok)
		++failures;
}

/* C23 7.16.1.4: va_start with no named parameter */
static int sum(...)
{
	va_list ap;

	va_start(ap);
	int total = 0;

	for (int value = va_arg(ap, int); value != 0; value = va_arg(ap, int))
		total += value;
	va_end(ap);
	return total;
}

/* The pre-C23 form with the last named parameter still works */
static long long sum_copy(int count, ...)
{
	va_list ap, copy;

	va_start(ap, count);
	va_copy(copy, ap);
	long long total = 0;

	for (int i = 0; i < count; ++i)
		total += va_arg(ap, long long) + va_arg(copy, long long);
	va_end(copy);
	va_end(ap);
	return total;
}

static int classify(int value)
{
	switch (value & 1) {
		case 0:
			return 0;
		case 1:
			return 1;
	}
	unreachable();
}

int main(void)
{
	check(sum(1, 2, 3, 4, 0) == 10, "va_start(ap) with no named parameters");
	check(sum_copy(3, 1LL, 2LL, 1LL << 40) == 2 * (3 + (1LL << 40)), "va_start(ap, last) and va_copy");
	check(classify(7) == 1 && classify(8) == 0, "unreachable() compiles in a void context");
	check(FLT_ROUNDS == 1, "FLT_ROUNDS is to nearest at startup");
	check((1 bitand 3) == 1 and (1 bitor 2) == 3 and not 0 and (5 xor 1) == 4, "iso646 spellings");
	check(NULL == nullptr, "NULL is a null pointer constant");

	return failures == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
