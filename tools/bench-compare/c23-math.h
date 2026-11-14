/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * c23-math.h
 *
 * Created on: Nov 14, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#ifndef _C23_MATH_H
#define _C23_MATH_H

/*
 * The C23 7.12 functions the math benchmark times that picolibc 1.8.6 and
 * newlib-nano do not provide (P11.06a). The comparison builds include this
 * header ahead of the benchmark source (math.c here) and link the placeholders of
 * c23-math.c; bench-runner drops their cases from the comparison
 * (COMPARE_ABSENT), so no placeholder figure is ever shown.
 */
float exp2m1f(float x);
double exp2m1(double x);
float exp10m1f(float x);
double exp10m1(double x);
float log2p1f(float x);
double log2p1(double x);
float log10p1f(float x);
double log10p1(double x);
float sinpif(float x);
double sinpi(double x);
float cospif(float x);
double cospi(double x);
float tanpif(float x);
double tanpi(double x);

#endif /* _C23_MATH_H */
