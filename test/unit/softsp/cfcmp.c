/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * cfcmp.c
 *
 * Created on: Oct 25, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include <stdint.h>
#include <string.h>

#include "softsp.h"

/*
 * The flag-returning comparisons (RTABI 4.1.2): the flags as for an integer
 * compare, Z (bit 30) for equal and C (bit 29) clear for less; unordered is
 * C set and Z clear. r2 and r3 must survive. r0 and r1 must survive
 * cfcmple and cfcmpeq; cfrcmple may leave them swapped, since libgcc,
 * compiler-rt and FreeBSD all swap them and tail-call cfcmple (soft-sp
 * swaps them back).
 */

static double widen(uint32_t bits)
{
	float x;

	memcpy(&x, &bits, sizeof(x));
	return x;
}

static uint32_t want_flags(uint32_t a, uint32_t b)
{
	volatile double x = widen(a);
	volatile double y = widen(b);

	if (x < y)
		return 0;
	if (x == y)
		return 0x60000000u;
	return 0x20000000u;
}

#define CALL_FLAGS(helper, a, b, flags, out0, out1, kept)                                                                                                 \
	do {                                                                                                                                                  \
		register uint32_t r0 __asm__("r0") = (a);                                                                                                         \
		register uint32_t r1 __asm__("r1") = (b);                                                                                                         \
		register uint32_t r2 __asm__("r2") = 0x2c2c2c2cu;                                                                                                 \
		register uint32_t r3 __asm__("r3") = 0x3d3d3d3du;                                                                                                 \
		__asm__ volatile("bl " helper "\n\tmrs %[out], apsr" : [out] "=r"(flags), "+r"(r0), "+r"(r1), "+r"(r2), "+r"(r3) : : "ip", "lr", "cc", "memory"); \
		(out0) = r0;                                                                                                                                      \
		(out1) = r1;                                                                                                                                      \
		(kept) = r2 == 0x2c2c2c2cu && r3 == 0x3d3d3d3du;                                                                                                  \
	} while (false)

void check_flag_compare(uint32_t a, uint32_t b)
{
	uint32_t flags;
	uint32_t first;
	uint32_t second;
	bool kept;

	CALL_FLAGS("__aeabi_cfcmple", a, b, flags, first, second, kept);
	if ((flags & 0x60000000u) != want_flags(a, b))
		fail("cfcmple", a, b);
	if (!kept || first != a || second != b)
		fail("cfcmple registers", a, b);

	CALL_FLAGS("__aeabi_cfcmpeq", a, b, flags, first, second, kept);
	if ((flags & 0x60000000u) != want_flags(a, b))
		fail("cfcmpeq", a, b);
	if (!kept || first != a || second != b)
		fail("cfcmpeq registers", a, b);

	CALL_FLAGS("__aeabi_cfrcmple", a, b, flags, first, second, kept);
	if ((flags & 0x60000000u) != want_flags(b, a))
		fail("cfrcmple", a, b);
	if (!kept || !((first == a && second == b) || (first == b && second == a)))
		fail("cfrcmple registers", a, b);
}
