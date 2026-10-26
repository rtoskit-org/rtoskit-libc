/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * __libc_log2f_table.c
 *
 * Created on: Oct 26, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include <stdint.h>

#include "libm-fixed.h"

/* The log2 reduction tables of the fixed-point cores (PLAN D86, D115): for c = 1 + j/32, j = -9 .. 13, 1/c as an unsigned Q31 number ... */
const uint32_t __libc_log2f_inverse[23] = {
	0xb21642c8, /* -9 */
	0xaaaaaaab, /* -8 */
	0xa3d70a3d, /* -7 */
	0x9d89d89e, /* -6 */
	0x97b425ed, /* -5 */
	0x92492492, /* -4 */
	0x8d3dcb09, /* -3 */
	0x88888889, /* -2 */
	0x84210842, /* -1 */
	0x80000000, /* 0 */
	0x7c1f07c2, /* 1 */
	0x78787878, /* 2 */
	0x75075075, /* 3 */
	0x71c71c72, /* 4 */
	0x6eb3e453, /* 5 */
	0x6bca1af3, /* 6 */
	0x69069069, /* 7 */
	0x66666666, /* 8 */
	0x63e7063e, /* 9 */
	0x61861862, /* 10 */
	0x5f417d06, /* 11 */
	0x5d1745d1, /* 12 */
	0x5b05b05b, /* 13 */
};

/* ... and log2 c' (c' = 1 over that number) in units of 2^-55 */
const int64_t __libc_log2f_logarithm[23] = {
	-0x03cfbebfc4ac049, /* -9 */
	-0x0351ff2e35e69da, /* -8 */
	-0x02d961ed04c9ccb, /* -7 */
	-0x02657fdc76c5c7d, /* -6 */
	-0x01f5fd8a8fab391, /* -5 */
	-0x018a8980a636815, /* -4 */
	-0x0122dadc2e4e9c9, /* -3 */
	-0x00beb024c09729a, /* -2 */
	-0x005dce5325f40ee, /* -1 */
	0x000000000000000, /* 0 */
	0x005aeb4dd4caa1a, /* 1 */
	0x00b31fb7e1d33c6, /* 2 */
	0x0108c588ce60487, /* 3 */
	0x015c01a399f816a, /* 4 */
	0x01acf5e2dc07738, /* 5 */
	0x01fbc16b832a888, /* 6 */
	0x024880f56279917, /* 7 */
	0x02934f09852e151, /* 8 */
	0x02dc4439c09d93f, /* 9 */
	0x032377511575947, /* 10 */
	0x0368fd7ee59effa, /* 11 */
	0x03acea7c14ca8e8, /* 12 */
	0x03ef50ad1a19841, /* 13 */
};
