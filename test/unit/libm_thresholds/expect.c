/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * expect.c
 *
 * Created on: Nov 17, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include "libm_thresholds.h"

/* The strict contract (debug, release, speed): its lgamma near the negative zeros is Q62 */
bool contract_lgamma_negative_zeros(void)
{
	return false;
}

/* The strict contract: below 0.53 ulp away from the negative zeros (D252) */
bool contract_lgamma_half_ulp(void)
{
	return true;
}
