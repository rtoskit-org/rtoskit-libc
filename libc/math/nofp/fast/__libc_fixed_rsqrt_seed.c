/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * __libc_fixed_rsqrt_seed.c
 *
 * Created on: Nov 12, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include <stdint.h>

#include "fixed-fast.h"

/* The seeds of __libc_fixed_square_root (fixed-fast.h), shared by the arcsine kernel and the hyperbolic logarithm */
const uint16_t __libc_fixed_rsqrt_seed[24] = { 31803, 30080, 28610, 27337, 26220, 25229, 24343, 23544, 22819, 22157, 21550, 20990, 20472, 19990, 19540, 19120, 18726, 18355, 18005, 17674, 17362, 17065, 16783, 16514 };
