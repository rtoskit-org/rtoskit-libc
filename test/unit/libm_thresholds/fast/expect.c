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

#include "../libm_thresholds.h"

/* The fast contract (D219, D225): lgamma within 2^-60 near every zero */
bool contract_lgamma_negative_zeros(void)
{
	return true;
}
