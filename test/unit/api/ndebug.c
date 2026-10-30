/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * ndebug.c
 *
 * Created on: Oct 30, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

/* C23 7.2p1: with NDEBUG defined where <assert.h> is included, assert is ((void)0) and its argument is not evaluated */
#define NDEBUG 1
#include <assert.h>

#include "api.h"

void ndebug_assert(void)
{
	assert(counted());
}
