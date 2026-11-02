/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * sincosf.c
 *
 * Created on: Nov 2, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#define _GNU_SOURCE

#include <math.h>

/*
 * GNU sincosf: the two results of sinf and cosf, each as accurate as the
 * function alone, so the architecture's libm layers serve both (PLAN D128)
 */
void sincosf(float x, float *sinp, float *cosp)
{
	*sinp = sinf(x);
	*cosp = cosf(x);
}
