/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * lgammaf.c
 *
 * Created on: Nov 17, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include <math.h>

/*
 * The fast lgammaf of every core but the single-precision FPU's (PLAN D162, D219, D229): the fast lgamma, rounded to
 * float. lgamma is within 0.53 ulp of double (and 2^-60 absolutely near the zeros, D225, D226), so the float result is
 * within 1 ulp, and almost always correctly rounded; its overflow from 4.08e36 is the conversion's. The strict lgammaf
 * rounds the same double but corrects two halfway cases (D98), which the fast contract does not need. C23 7.12.8.3.
 */
float lgammaf(float x)
{
	return (float)lgamma(x);
}
