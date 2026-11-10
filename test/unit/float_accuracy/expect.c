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
 * The accuracy of the float functions in debug, release and speed (PLAN D57, D62): every result below 1 ulp, the
 * worst below 0.52 ulp (faithful with margin), and at most one result in ten thousand, plus one, not correctly
 * rounded
 */

#include "float_accuracy.h"

double float_accuracy_tolerance([[maybe_unused]] const char *name, [[maybe_unused]] double exact)
{
	return 1.0;
}

bool float_accuracy_within([[maybe_unused]] const char *name, double worst, long misrounded, long count)
{
	return worst < 0.52 && misrounded <= count / 10000 + 1;
}
