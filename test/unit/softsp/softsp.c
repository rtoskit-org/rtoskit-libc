/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * softsp.c
 *
 * Created on: Oct 25, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include <stdint.h>
#include <stdio.h>

#include "softsp.h"

/*
 * The single-precision run-time ABI helpers (RTABI 4.1.2; PLAN D109):
 * soft-sp's Thumb-2 assembly on armv7-m, libgcc's or nofp-thumb1's on the
 * other Arm cores, against references that never compute in float.
 */

static int failures;

void fail(const char *what, uint32_t a, uint32_t b)
{
	char message[128];

	if (failures < 20) {
		snprintf(message, sizeof(message), "fail: %s 0x%08lx 0x%08lx\n", what, (unsigned long)a, (unsigned long)b);
		fputs(message, stdout);
	}
	++failures;
}

/* xorshift64: deterministic test values */
uint64_t next(uint64_t *state)
{
	*state ^= *state << 13;
	*state ^= *state >> 7;
	*state ^= *state << 17;
	return *state;
}

int main(void)
{
	test_operations();
	if (failures != 0) {
		char message[64];

		snprintf(message, sizeof(message), "softsp: %d failures\n", failures);
		fputs(message, stdout);
		return 1;
	}
	fputs("softsp: pass\n", stdout);
	return 0;
}
