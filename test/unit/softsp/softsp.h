/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * softsp.h
 *
 * Created on: Oct 25, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#ifndef _SOFTSP_H
#define _SOFTSP_H

#include <stdint.h>

/* Shared by the test's sources */
void fail(const char *what, uint32_t a, uint32_t b);
uint64_t next(uint64_t *state);

/* The compiler's helpers that call the float helpers (soft-sp's on armv7-m, libgcc's elsewhere); the compiler's prototypes */
_Complex float __mulsc3(float a, float b, float c, float d);
float __powisf2(float x, int n);

/* The single-precision run-time ABI helpers against references (operations.c on Arm, none.c elsewhere) */
void test_operations(void);

/* The flag-returning comparisons, __aeabi_cfcmple, cfcmpeq and cfrcmple (cfcmp.c; cfcmp-thumb1.c on Armv6-M) */
void check_flag_compare(uint32_t a, uint32_t b);

#endif /* _SOFTSP_H */
