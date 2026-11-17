/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * tgammaf.c
 *
 * Created on: Nov 17, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include <math.h>

/*
 * The fast tgammaf of the soft-float cores (PLAN D162, D219, D232; the generic fast file's, here so that it beats
 * sp-nearest's strict float core, which these cores list as an earlier layer): the fast tgamma, rounded to
 * float. tgamma is within 0.59 ulp of double (D230, D231), so the float result is
 * within 1 ulp, and almost always correctly rounded; its overflow from 35.0401 and its underflow are the conversion's. The strict tgammaf
 * rounds the same double but corrects two halfway cases (D98), which the fast contract does not need. C23 7.12.8.4.
 */
float tgammaf(float x)
{
	return (float)tgamma(x);
}
