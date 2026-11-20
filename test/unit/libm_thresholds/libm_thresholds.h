/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * libm_thresholds.h
 *
 * Created on: Nov 17, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#ifndef _LIBM_THRESHOLDS_H
#define _LIBM_THRESHOLDS_H

#include <stdbool.h>

/*
 * Whether the build type's lgamma keeps 2^-60 absolutely near its negative zeros (D219, D225): the fast file does; the
 * strict file is about 2^-57 off there and misrounds lgamma(-2.5) (Q62), a known defect outside phase 11 (D162)
 */
bool contract_lgamma_negative_zeros(void);

/* Whether the build type's lgamma is below 0.53 ulp away from its negative zeros (the strict contract, D252); fast's bound is 1 ulp */
bool contract_lgamma_half_ulp(void);

#endif /* _LIBM_THRESHOLDS_H */
