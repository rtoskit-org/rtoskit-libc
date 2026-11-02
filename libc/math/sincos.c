/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * sincos.c
 *
 * Created on: Nov 2, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#define _GNU_SOURCE

#include <math.h>

/*
 * GNU sincos: the two results of sin and cos, each as accurate as the
 * function alone, so the architecture's libm layers serve both (PLAN D128)
 */
void sincos(double x, double *sinp, double *cosp)
{
	*sinp = sin(x);
	*cosp = cos(x);
}
