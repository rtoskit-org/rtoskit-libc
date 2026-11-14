/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * c23-math.c
 *
 * Created on: Nov 14, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include "c23-math.h"

/*
 * Placeholders for the functions the compared libraries lack, so that the
 * math benchmark links; they return their argument and their cases are left
 * out of the comparison (bench-runner COMPARE_ABSENT)
 */
float exp2m1f(float x)
{
	return x;
}

double exp2m1(double x)
{
	return x;
}

float exp10m1f(float x)
{
	return x;
}

double exp10m1(double x)
{
	return x;
}

float log2p1f(float x)
{
	return x;
}

double log2p1(double x)
{
	return x;
}

float log10p1f(float x)
{
	return x;
}

double log10p1(double x)
{
	return x;
}

float sinpif(float x)
{
	return x;
}

double sinpi(double x)
{
	return x;
}

float cospif(float x)
{
	return x;
}

double cospi(double x)
{
	return x;
}

float tanpif(float x)
{
	return x;
}

double tanpi(double x)
{
	return x;
}
