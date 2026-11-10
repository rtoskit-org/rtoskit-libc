/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * libm_dense.h
 *
 * Created on: Nov 10, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#ifndef _LIBM_DENSE_H
#define _LIBM_DENSE_H

/*
 * What the build type accepts of the dense sweeps and the near-halfway cases (expect.c in debug, release and speed,
 * fast/expect.c in fast, D162, D164, selected by subdir.mk): whether a float result of name is right against its
 * reference (the double function), how many wrong results a sweep of count allows, and whether a near-halfway
 * result is acceptable, float (want the correctly rounded result) or double
 */
bool libm_dense_accept(const char *name, float got, double reference);
unsigned long libm_dense_allowance(const char *name, unsigned long count);
bool libm_dense_near(const char *name, float got, float want, double reference);
bool libm_dense_near_double(const char *name, double got, double want);

#endif /* _LIBM_DENSE_H */
