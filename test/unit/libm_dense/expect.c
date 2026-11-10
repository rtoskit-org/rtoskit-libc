/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * expect.c
 *
 * Created on: Nov 10, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

/*
 * The dense sweeps' and near-halfway cases' expectation in debug, release and speed (PLAN D57, D86-D88): the double
 * function rounded to float, but for one result in ten thousand, plus one, in a sweep; every near-halfway result
 * correctly rounded
 */

#include <string.h>

#include "libm_dense.h"

bool libm_dense_accept([[maybe_unused]] const char *name, float got, double reference)
{
	float want = (float)reference;

	return memcmp(&got, &want, sizeof(got)) == 0;
}

unsigned long libm_dense_allowance([[maybe_unused]] const char *name, unsigned long count)
{
	return count / 10000 + 1;
}

bool libm_dense_near([[maybe_unused]] const char *name, float got, float want, [[maybe_unused]] double reference)
{
	return memcmp(&got, &want, sizeof(got)) == 0;
}

bool libm_dense_near_double([[maybe_unused]] const char *name, double got, double want)
{
	return memcmp(&got, &want, sizeof(got)) == 0;
}
