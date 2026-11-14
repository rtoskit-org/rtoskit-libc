/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * math.c
 *
 * Created on: Nov 14, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

/*
 * The math benchmark as the comparison builds compile it (P11.06a): the
 * declarations of the C23 functions picolibc and newlib-nano lack, then the
 * benchmark source unchanged
 */
#include "c23-math.h"
#include "../../test/bench/math/math.c"
