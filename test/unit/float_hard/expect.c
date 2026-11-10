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

/* The hard cases' expectation in debug, release and speed (PLAN D98): the correctly rounded result in every mode */

#include <string.h>

#include "float_hard.h"

bool float_hard_accept([[maybe_unused]] const char *name, [[maybe_unused]] bool nearest, float got, float want, [[maybe_unused]] float result, [[maybe_unused]] int direction)
{
	return memcmp(&got, &want, sizeof(got)) == 0;
}
