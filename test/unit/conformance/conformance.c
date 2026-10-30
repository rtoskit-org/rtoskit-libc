/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * conformance.c
 *
 * Created on: Oct 30, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

/*
 * The behaviours libc-test's functional/ and regression/ tests checked that
 * the other test/unit modules did not (issue #94; the mapping is in
 * docs/research/conformance.md). Each regression case is named after the bug
 * class it guards. Inputs and expected values are derived from the standard
 * text (N3220) and computed independently: the decimal rounding boundaries
 * with Python's decimal module at 2000 digits, the division results from
 * the identity q * y + r == x.
 */

#include <complex.h>
#include <errno.h>
#include <fenv.h>
#include <float.h>
#include <inttypes.h>
#include <limits.h>
#include <math.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <tgmath.h>
#include <time.h>

static int failures;

static void fail(int line, const char *what)
{
	printf("FAIL line %d: %s\n", line, what);
	++failures;
}

#define CHECK(condition)                \
	do {                                \
		if (!(condition))               \
			fail(__LINE__, #condition); \
	} while (false)

/* A formatted result and its length (C23 7.23.6.5p3) */
#define CHECK_FORMAT(want, format, ...)                                                                \
	do {                                                                                               \
		char text[512];                                                                                \
		int length = snprintf(text, sizeof(text), format, __VA_ARGS__);                                \
		if (length != (int)strlen(want) || strcmp(text, want) != 0) {                                  \
			char message[700];                                                                         \
			snprintf(message, sizeof(message), "\"%s\" gave \"%s\", want \"%s\"", format, text, want); \
			fail(__LINE__, message);                                                                   \
		}                                                                                              \
	} while (false)

static uint64_t random_state = 0x9e3779b97f4a7c15u;

/* A 64-bit PCG-style step: deterministic inputs, nothing from libc-test */
static uint64_t next_random(void)
{
	random_state = random_state * 6364136223846793005u + 1442695040888963407u;
	return random_state ^ (random_state >> 29);
}

/* regression/malloc-0: malloc(0) returns null or a unique pointer that free accepts (C23 7.24.3p1) */
static void regression_malloc_zero(void)
{
	void *pointers[4];

	for (int i = 0; i < 4; ++i)
		pointers[i] = malloc(0);
	for (int i = 0; i < 4; ++i)
		for (int j = i + 1; j < 4; ++j)
			CHECK(pointers[i] == nullptr || pointers[i] != pointers[j]);
	for (int i = 0; i < 4; ++i)
		free(pointers[i]);
}

/* regression/malloc-oom: a request the heap cannot satisfy returns null with ENOMEM (test/unit/malloc exhausts the heap itself) */
static void regression_malloc_out_of_memory(void)
{
	errno = 0;
	CHECK(malloc(SIZE_MAX / 2) == nullptr && errno == ENOMEM);
	errno = 0;
	CHECK(calloc(SIZE_MAX / 4, 4) == nullptr && errno == ENOMEM);
}

/* regression/printf-1e9-oob: rounding that carries into a new leading digit, and long fixed output */
static void regression_printf_carry(void)
{
	CHECK_FORMAT("1e+09", "%g", 999999999.5);
	CHECK_FORMAT("1.000e+04", "%.3e", 9999.9996);
	CHECK_FORMAT("98765432109876.0", "%.1f", 98765432109876.0);
	CHECK_FORMAT("100", "%.0f", 99.5);
}

/* regression/printf-fmt-g-round: %g rounds at the precision, then drops trailing zeros; an exact tie rounds to even */
static void regression_printf_g_round(void)
{
	CHECK_FORMAT("5e+12", "%.12g", 4999999999999.6);
	CHECK_FORMAT("3.00000000004e+14", "%.12g", 300000000004500.0);
	CHECK_FORMAT("3.00000000006e+14", "%.12g", 300000000005500.0);
}

/* regression/printf-fmt-g-zeros: a large %g precision prints the exact value without trailing zeros */
static void regression_printf_g_zeros(void)
{
	CHECK_FORMAT("123456789012345.25", "%.40g", 123456789012345.25);
	CHECK_FORMAT("500000000000000", "%.40g", 5e14);
	CHECK_FORMAT("0.5", "%.40g", 0.5);
}

/* regression/printf-fmt-n: %n with every length modifier counts what was written so far (C23 7.23.6.1p8) */
static void regression_printf_n(void)
{
	char text[512];
	signed char count_hh = 0;
	short count_h = 0;
	int count = 0;
	long count_l = 0;
	long long count_ll = 0;
	intmax_t count_j = 0;
	size_t count_z = 0;
	ptrdiff_t count_t = 0;

	CHECK(snprintf(text, sizeof(text), "%300d%d%n%d", 1, 2, &count, 3) == 302 && count == 301);
	CHECK(snprintf(text, sizeof(text), "%300d%d%hn%d", 1, 2, &count_h, 3) == 302 && count_h == 301);
	CHECK(snprintf(text, sizeof(text), "%100d%d%hhn%d", 1, 2, &count_hh, 3) == 102 && count_hh == 101);
	CHECK(snprintf(text, sizeof(text), "%300d%d%ln%d", 1, 2, &count_l, 3) == 302 && count_l == 301);
	CHECK(snprintf(text, sizeof(text), "%300d%d%lln%d", 1, 2, &count_ll, 3) == 302 && count_ll == 301);
	CHECK(snprintf(text, sizeof(text), "%300d%d%jn%d", 1, 2, &count_j, 3) == 302 && count_j == 301);
	CHECK(snprintf(text, sizeof(text), "%300d%d%zn%d", 1, 2, &count_z, 3) == 302 && count_z == 301);
	CHECK(snprintf(text, sizeof(text), "%300d%d%tn%d", 1, 2, &count_t, 3) == 302 && count_t == 301);
}

/* regression/scanf-bytes-consumed: %n after %s counts every character the string consumed */
static void regression_scanf_count_after_string(void)
{
	char word[8];
	int consumed = -1;

	CHECK(sscanf("xyz", "%s%n", word, &consumed) == 1 && consumed == 3 && strcmp(word, "xyz") == 0);
	CHECK(sscanf("  xyz  ", "%s%n", word, &consumed) == 1 && consumed == 5);
}

/* regression/scanf-match-literal-eof: input failure before the first conversion returns EOF (C23 7.23.6.2p16) */
static void regression_scanf_literal_at_eof(void)
{
	int value = 0;

	CHECK(sscanf("", "q") == EOF);
	CHECK(sscanf("", "q%d", &value) == EOF);
	CHECK(sscanf("r", "q%d", &value) == 0);
}

/* regression/scanf-nullbyte-char: %c stores exactly the characters matched and no null (C23 7.23.6.2p12) */
static void regression_scanf_char_no_null(void)
{
	char letters[4] = { '#', '#', '#', '#' };

	CHECK(sscanf("pq", "%c", letters) == 1 && letters[0] == 'p' && letters[1] == '#');
	CHECK(sscanf("pqrs", "%3c", letters) == 1 && memcmp(letters, "pqr#", 4) == 0);
}

/* regression/setvbuf-unget: pushing back characters never writes outside the buffer given to setvbuf */
static void regression_setvbuf_ungetc(void)
{
	char space[96];
	FILE *stream = tmpfile();

	CHECK(stream != nullptr);
	if (stream == nullptr)
		return;
	memset(space, 'g', sizeof(space));
	CHECK(setvbuf(stream, space + 16, _IOFBF, 64) == 0);
	CHECK(fputs("data", stream) >= 0 && fseek(stream, 0, SEEK_SET) == 0 && fgetc(stream) == 'd');
	for (int i = 0; i < 200 && ungetc('u', stream) != EOF; ++i);
	for (size_t i = 0; i < 16; ++i)
		CHECK(space[i] == 'g' && space[80 + i] == 'g');
	fclose(stream);
}

/* regression/sscanf-eof: a conversion that meets the end of input stops the scan; later objects are untouched */
static void regression_sscanf_float_at_end(void)
{
	float number = 9.0f;
	char letter = '#';

	CHECK(sscanf("7", "%f%c", &number, &letter) == 1 && number == 7.0f && letter == '#');
	CHECK(sscanf("2.5e", "%f%c", &number, &letter) == 0 && letter == '#');
}

/* regression/fpclassify-invalid-ld80: long double is binary64 here (D7); every class and %La of each */
static void regression_long_double_classes(void)
{
	CHECK(fpclassify(0.0L) == FP_ZERO && fpclassify(LDBL_TRUE_MIN) == FP_SUBNORMAL && fpclassify(1.0L) == FP_NORMAL);
	CHECK(fpclassify((long double)INFINITY) == FP_INFINITE && fpclassify((long double)NAN) == FP_NAN);
	CHECK_FORMAT("0x1p+0", "%La", 1.0L);
	CHECK_FORMAT("-inf", "%La", -(long double)INFINITY);
}

/* Soft-double cores: (float) of a NaN set the sign from the double's exponent bits (issue #95's vectors found it) */
static void regression_nan_conversion_sign(void)
{
	volatile double positive = NAN;
	volatile double negative = -(double)NAN;
	float narrowed;

	narrowed = (float)positive;
	CHECK(isnan(narrowed) && !signbit(narrowed));
	CHECK(copysignf(1.0f, narrowed) == 1.0f);
	narrowed = (float)negative;
	CHECK(isnan(narrowed) && signbit(narrowed));
	CHECK(!signbit((float)(volatile double){ __builtin_nan("0x123456789") }));
}

/*
 * The libm vectors' directed-rounding findings (Q36): cbrt put the sign on
 * after rounding, so upward and downward swapped for a negative x, and its
 * 0.6 ulp to nearest; the float helper for tiny arguments compared signbit of
 * a float with signbit of a double; tgamma dropped -gamma beside 1/x; pow
 * formed y log|x| in the caller's direction
 */
static void regression_directed_rounding(void)
{
	CHECK(cbrt(0x1.666336999764ep+6) == 0x1.1e618cb93c594p+2);
#ifdef FE_DOWNWARD
	CHECK(fesetround(FE_UPWARD) == 0);
	CHECK(cbrt(-0x1.1b84085b38e0dp+6) == -0x1.08dc938bd55e2p+2);
	CHECK(pow(2.0, 0x1p-1074) == 0x1.0000000000001p+0);
	CHECK(fesetround(FE_DOWNWARD) == 0);
	CHECK(cbrt(-0x1.1b84085b38e0dp+6) == -0x1.08dc938bd55e3p+2);
	CHECK(asinf(-0x1p-126f) == -0x1.000002p-126f);
	CHECK(log1pf(-0x1p-126f) == -0x1.000002p-126f);
	CHECK(tgammaf(0x1p-126f) == 0x1.fffffep+125f);
	CHECK(tgamma(0x1p-126) == 0x1.fffffffffffffp+125);
	CHECK(pow(2.0, 0x1p-1074) == 1.0);
	CHECK(fesetround(FE_TONEAREST) == 0);
#endif
}

/* functional/strtod, strtod_long, strtod_simple, strtold: exact rounding at the boundaries, very long input, round trips */
static void functional_strtod(void)
{
	static const struct {
		const char *text;
		double want;
	} doubles[] = {
		/* Between 0 and 2^-1074: below the midpoint, the midpoint itself (ties to even), above it */
		{ "2.47032822920623272088284396434110686182529901307162382212792e-324", 0.0 },
		{ "2.47032822920623272088284396434110686182529901307162382212793e-324", 0x1p-1074 },
		/* Between 2^-1074 and 2^-1073 */
		{ "7.41098468761869816264853189302332058547589703921487146638378e-324", 0x1p-1074 },
		{ "7.41098468761869816264853189302332058547589703921487146638379e-324", 0x1p-1073 },
		/* Between the largest subnormal and DBL_MIN */
		{ "2.22507385850720113605740979670913197593481954635164564802342e-308", 0x0.fffffffffffffp-1022 },
		{ "2.22507385850720113605740979670913197593481954635164564802343e-308", 0x1p-1022 },
		/* Between DBL_MAX and the overflow threshold */
		{ "1.79769313486231580793728971405303415079934132710037826936173e+308", DBL_MAX },
		{ "1.79769313486231580793728971405303415079934132710037826936174e+308", INFINITY },
		/* Hexadecimal: the bits beyond the significand round to nearest, ties to even */
		{ "0x1p4", 16.0 },
		{ "0x1.1p4", 17.0 },
		{ "0x1.00000000000008", 1.0 },
		{ "0x1.000000000000080000001", 0x1.0000000000001p0 },
		{ "0x1.00000000000018", 0x1.0000000000002p0 },
		{ "-.00000", -0.0 },
		{ "1e+100000", INFINITY },
		{ "1e-100000", 0.0 },
	};
	static const struct {
		const char *text;
		float want;
	} floats[] = {
		{ "7.00649232162408535461864791644958065640130970938257885878534e-46", 0.0f },
		{ "7.00649232162408535461864791644958065640130970938257885878535e-46", 0x1p-149f },
		{ "2.10194769648722560638559437493487419692039291281477365763560e-45", 0x1p-149f },
		{ "2.10194769648722560638559437493487419692039291281477365763561e-45", 0x1p-148f },
		{ "1.17549428075736429172788299103576651332285899275899042768296e-38", 0x0.fffffep-126f },
		{ "1.17549428075736429172788299103576651332285899275899042768297e-38", 0x1p-126f },
		/* FLT_MAX plus half its ulp is an integer: just below it stays FLT_MAX, the tie itself overflows (ties to even) */
		{ "340282356779733661637539395458142568447.999999", FLT_MAX },
		{ "340282356779733661637539395458142568448", INFINITY },
	};
	char text[512];
	char *end;

	for (size_t i = 0; i < sizeof(doubles) / sizeof(doubles[0]); ++i) {
		double value = strtod(doubles[i].text, &end);

		if (memcmp(&value, &doubles[i].want, sizeof(value)) != 0 || *end != '\0') {
			snprintf(text, sizeof(text), "strtod(\"%s\") = %a, want %a", doubles[i].text, value, doubles[i].want);
			fail(__LINE__, text);
		}
		CHECK(strtold(doubles[i].text, nullptr) == (long double)doubles[i].want);
	}
	for (size_t i = 0; i < sizeof(floats) / sizeof(floats[0]); ++i) {
		float value = strtof(floats[i].text, &end);

		if (value != floats[i].want || *end != '\0') {
			snprintf(text, sizeof(text), "strtof(\"%s\") = %a, want %a", floats[i].text, (double)value, (double)floats[i].want);
			fail(__LINE__, text);
		}
	}

	/* 40000 significant digits: the value is within 10^-40000 of 1/9, so it rounds as 1/9 does */
	char *long_text = malloc(40002);

	CHECK(long_text != nullptr);
	if (long_text != nullptr) {
		long_text[0] = '.';
		memset(long_text + 1, '1', 40000);
		long_text[40001] = '\0';
		CHECK(strtod(long_text, &end) == 1.0 / 9.0 && end == long_text + 40001);
		free(long_text);
	}

	/* %.300f keeps every digit that matters: strtod returns the same double */
	for (int i = 0; i < 100; ++i) {
		double value = (double)(next_random() >> 11) * 0x1p-53 * 1000.0 + 0.001;

		snprintf(text, sizeof(text), "%.300f", value);
		CHECK(strtod(text, nullptr) == value);
	}
}

/* functional/strtol: the limits of long and unsigned long, whatever their width, and negation in strtoul */
static void functional_strtol(void)
{
	char text[64];
	char *end;

	/* One past LONG_MAX: the last digit of LONG_MAX is 7 for every two's complement width */
	snprintf(text, sizeof(text), "%ld", LONG_MAX);
	text[strlen(text) - 1] += 1;
	errno = 0;
	CHECK(strtol(text, &end, 0) == LONG_MAX && errno == ERANGE && *end == '\0');
	snprintf(text, sizeof(text), "-%lu", (unsigned long)LONG_MAX + 2);
	errno = 0;
	CHECK(strtol(text, &end, 0) == LONG_MIN && errno == ERANGE && *end == '\0');
	snprintf(text, sizeof(text), "%lu", ULONG_MAX);
	text[strlen(text) - 1] += 1;
	errno = 0;
	CHECK(strtoul(text, &end, 0) == ULONG_MAX && errno == ERANGE && *end == '\0');

	/* C23 7.24.1.7p5: a minus sign negates in the return type, with no range error */
	errno = 0;
	CHECK(strtoul("-1", &end, 0) == ULONG_MAX && errno == 0 && *end == '\0');
	CHECK(strtoul("-2", nullptr, 0) == ULONG_MAX - 1 && errno == 0);
	snprintf(text, sizeof(text), "-%ld", LONG_MAX);
	CHECK(strtoul(text, nullptr, 0) == (unsigned long)LONG_MAX + 2 && errno == 0);
	CHECK(atol("2147483647") == 2147483647L && strtoul("4294967295", nullptr, 0) == 4294967295UL);
}

/* functional/sscanf: scansets, %c widths, field widths, assignment suppression and %i prefixes */
static void functional_sscanf(void)
{
	char first[16];
	char second[16] = "#";
	int a = 0;
	int b = 0;
	int c = 0;
	unsigned int d = 0;

	CHECK(sscanf("abba ok", "%[ab]%s", first, second) == 2 && strcmp(first, "abba") == 0 && strcmp(second, "ok") == 0);
	CHECK(sscanf("abbc, ok", "%[ab] %s", first, second) == 2 && strcmp(first, "abb") == 0 && strcmp(second, "c,") == 0);
	/* The second %5c meets the end after three characters: an input failure, so one item; what it stored is unspecified */
	CHECK(sscanf("abcdefgh", "%5c%5c", first, second) == 1 && memcmp(first, "abcde", 5) == 0);
	CHECK(sscanf("12345 678 9x", "%2d%d%*d %[0-9]", &a, &b, first) == 3 && a == 12 && b == 345 && strcmp(first, "9") == 0);
	CHECK(sscanf("017 0x1f 17 1f", "%i %i %o %x", &a, &b, &c, &d) == 4 && a == 15 && b == 31 && c == 15 && d == 31);
	CHECK(sscanf("-0x10", "%i", &a) == 1 && a == -16);
}

static int compare_int(const void *left, const void *right)
{
	int x = *(const int *)left;
	int y = *(const int *)right;

	return (x > y) - (x < y);
}

static int compare_u64(const void *left, const void *right)
{
	uint64_t x = *(const uint64_t *)left;
	uint64_t y = *(const uint64_t *)right;

	return (x > y) - (x < y);
}

static int compare_char(const void *left, const void *right)
{
	return *(const unsigned char *)left - *(const unsigned char *)right;
}

static int compare_string(const void *left, const void *right)
{
	return strcmp(*(const char *const *)left, *(const char *const *)right);
}

/* functional/qsort: odd and prime lengths, 64-bit keys, strings, and every permutation of a short array */
static void functional_qsort(void)
{
	int numbers[31];
	uint64_t keys[101];
	const char *words[] = { "pear", "fig", "apple", "kiwi", "date", "lime", "plum", "apple", "yuzu", "cherry", "grape" };
	char permutation[6];
	bool sorted = true;

	for (int i = 0; i < 31; ++i)
		numbers[i] = (int)(next_random() % 41) - 20;
	qsort(numbers, 31, sizeof(numbers[0]), compare_int);
	for (int i = 1; i < 31; ++i)
		sorted = sorted && numbers[i - 1] <= numbers[i];
	CHECK(sorted);

	for (int i = 0; i < 101; ++i)
		keys[i] = (i % 7 == 0) ? keys[i / 2] : next_random();
	qsort(keys, 101, sizeof(keys[0]), compare_u64);
	for (int i = 1; i < 101; ++i)
		sorted = sorted && keys[i - 1] <= keys[i];
	CHECK(sorted);

	qsort(words, sizeof(words) / sizeof(words[0]), sizeof(words[0]), compare_string);
	for (size_t i = 1; i < sizeof(words) / sizeof(words[0]); ++i)
		sorted = sorted && strcmp(words[i - 1], words[i]) <= 0;
	CHECK(sorted);

	/* Every permutation of "abcde" (Heap's algorithm, iterative) sorts to "abcde" */
	char base[6] = "abcde";
	int counters[5] = {};
	int index = 1;
	int permutations = 1;

	memcpy(permutation, base, 6);
	qsort(permutation, 5, 1, compare_char);
	sorted = strcmp(permutation, "abcde") == 0;
	while (index < 5) {
		if (counters[index] < index) {
			int other = (index % 2 == 0) ? 0 : counters[index];
			char swap = base[other];

			base[other] = base[index];
			base[index] = swap;
			memcpy(permutation, base, 6);
			qsort(permutation, 5, 1, compare_char);
			sorted = sorted && strcmp(permutation, "abcde") == 0;
			++permutations;
			++counters[index];
			index = 1;
		} else {
			counters[index] = 0;
			++index;
		}
	}
	CHECK(sorted && permutations == 120);
}

/* functional/ungetc: the position indicator and the pushed-back character through ftell, scansets, fread and fseek */
static void functional_ungetc(void)
{
	char text[32];
	FILE *stream = tmpfile();

	CHECK(stream != nullptr);
	if (stream == nullptr)
		return;
	CHECK(fputs("abc\n", stream) >= 0 && fseek(stream, 0, SEEK_SET) == 0);
	CHECK(fgetc(stream) == 'a' && ftell(stream) == 1);
	/* C23 7.23.7.10p5: on a binary stream each pushback decrements the position */
	CHECK(ungetc('z', stream) == 'z' && ftell(stream) == 0);
	CHECK(fscanf(stream, "%[a]", text) == 0 && ftell(stream) == 0);
	CHECK(fgetc(stream) == 'z' && ftell(stream) == 1);
	/* p5: a successful fseek discards pushed-back characters */
	CHECK(fseek(stream, 0, SEEK_SET) == 0 && ungetc('z', stream) == 'z');
	CHECK(fread(text, 1, sizeof(text), stream) == 5 && memcmp(text, "zabc\n", 5) == 0);
	CHECK(fseek(stream, 0, SEEK_SET) == 0 && fscanf(stream, "%[q]", text) == 0);
	CHECK(ungetc('y', stream) == 'y' && fgetc(stream) == 'y' && fgetc(stream) == 'a');
	fclose(stream);
}

/* functional/tgmath: the argument type selects the function, so float rounding shows through */
static void functional_tgmath(void)
{
	/* 33554435 is not a float: 33554435.0f is 33554436, and lrint keeps each */
	CHECK(lrint(33554435.0f) == 33554436 && lrint(33554435.0) == 33554435);
	CHECK(sqrt(2.0f) == 0x1.6a09e6p0f && sqrt(2.0) == 0x1.6a09e667f3bcdp0 && sqrt(2) == 0x1.6a09e667f3bcdp0);
	CHECK(sizeof(pow(2.0f, 0.5f)) == sizeof(float) && sizeof(pow(2.0, 0.5f)) == sizeof(double) && sizeof(pow(2, 0.5f)) == sizeof(double));
	CHECK(sizeof(pow(2.0, 0.5 + 0 * I)) == sizeof(double complex));
	CHECK(pow(2, 0.5) == 0x1.6a09e667f3bcdp0 && pow(2.0f, 0.5f) == 0x1.6a09e6p0f);
}

/* functional/udiv: 64-bit / and % satisfy q * y + r == x with 0 <= r < y, and truncate towards zero when signed */
static void functional_division(void)
{
	bool exact = true;

	for (int i = 0; i < 256; ++i) {
		uint64_t x = next_random();
		uint64_t y = next_random() >> (i / 4);
		int64_t sx = (int64_t)next_random() >> (i % 32);
		int64_t sy = (int64_t)next_random() >> (i / 4 + 1);

		if (y != 0) {
			uint64_t q = x / y;
			uint64_t r = x % y;

			exact = exact && q * y + r == x && r < y;
		}
		if (sy != 0 && !(sx == INT64_MIN && sy == -1)) {
			int64_t q = sx / sy;
			int64_t r = sx % sy;

			exact = exact && q * sy + r == sx && (r == 0 || (r < 0) == (sx < 0)) && (r < 0 ? -r : r) < (sy < 0 ? -(uint64_t)sy : (uint64_t)sy);
		}
	}
	CHECK(exact);
	CHECK(UINT64_MAX / 3 == 0x5555555555555555u && UINT64_MAX % 10 == 5 && INT64_MIN / 2 == -0x4000000000000000 && -7 % 3 == -1);
}

/* functional/time: gmtime and timegm invert each other across a wide range of times */
static void functional_time(void)
{
	bool exact = true;

	for (int i = 0; i < 400; ++i) {
		time_t t = (time_t)((int64_t)(next_random() >> 22) - ((int64_t)1 << 41));
		struct tm broken;

		if (sizeof(time_t) < 8)
			t = (time_t)(int32_t)t;
		exact = exact && gmtime_r(&t, &broken) != nullptr && timegm(&broken) == t;
		exact = exact && broken.tm_sec >= 0 && broken.tm_sec <= 59 && broken.tm_min <= 59 && broken.tm_hour <= 23 && broken.tm_mday >= 1 && broken.tm_mday <= 31 && broken.tm_mon <= 11 && broken.tm_wday <= 6 && broken.tm_yday <= 365;
	}
	CHECK(exact);
}

/* functional/strftime: a five-digit year, the year 0 and the 12-hour clock at midnight and noon */
static void functional_strftime(void)
{
	char text[128];
	struct tm tm = { .tm_year = 10009 - 1900, .tm_mon = 0, .tm_mday = 5, .tm_hour = 0, .tm_min = 17, .tm_sec = 53, .tm_wday = 1, .tm_yday = 4 };

	/* Plain %Y is POSIX %+4Y, so a five-digit year carries a sign (libc/time/strftime.c) */
	CHECK(strftime(text, sizeof(text), "%Y|%C|%y|%r", &tm) > 0 && strcmp(text, "+10009|100|09|12:17:53 AM") == 0);
	tm = (struct tm){ .tm_year = -1900, .tm_mon = 1, .tm_mday = 23, .tm_hour = 12, .tm_wday = 3, .tm_yday = 53 };
	CHECK(strftime(text, sizeof(text), "%C|%y|%r|%a %b", &tm) > 0 && strcmp(text, "00|00|12:00:00 PM|Wed Feb") == 0);
}

int main(void)
{
	regression_malloc_zero();
	regression_malloc_out_of_memory();
	regression_printf_carry();
	regression_printf_g_round();
	regression_printf_g_zeros();
	regression_printf_n();
	regression_scanf_count_after_string();
	regression_scanf_literal_at_eof();
	regression_scanf_char_no_null();
	regression_setvbuf_ungetc();
	regression_sscanf_float_at_end();
	regression_long_double_classes();
	regression_nan_conversion_sign();
	regression_directed_rounding();

	functional_strtod();
	functional_strtol();
	functional_sscanf();
	functional_qsort();
	functional_ungetc();
	functional_tgmath();
	functional_division();
	functional_time();
	functional_strftime();

	printf("%s\n", failures == 0 ? "conformance: ok" : "conformance: failed");
	return failures == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
