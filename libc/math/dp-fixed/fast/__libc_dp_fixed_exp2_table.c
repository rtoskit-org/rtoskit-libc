/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * __libc_dp_fixed_exp2_table.c
 *
 * Created on: Nov 12, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include <stdint.h>

#include "dp-fixed-fast.h"

/* 2^(j/64) 2^63 rounded to nearest, j = 0..63, computed with 300-bit arithmetic (docs/research/libm-fast-double.md, section 7) */
const uint64_t __libc_dp_fixed_exp2_table[64] = {
	0x8000000000000000ull, /* j = 0 */
	0x8164d1f3bc030773ull, /* j = 1 */
	0x82cd8698ac2ba1d7ull, /* j = 2 */
	0x843a28c3acde4046ull, /* j = 3 */
	0x85aac367cc487b15ull, /* j = 4 */
	0x871f61969e8d1010ull, /* j = 5 */
	0x88980e8092da8527ull, /* j = 6 */
	0x8a14d575496efd9aull, /* j = 7 */
	0x8b95c1e3ea8bd6e7ull, /* j = 8 */
	0x8d1adf5b7e5ba9e6ull, /* j = 9 */
	0x8ea4398b45cd53c0ull, /* j = 10 */
	0x9031dc431466b1dcull, /* j = 11 */
	0x91c3d373ab11c336ull, /* j = 12 */
	0x935a2b2f13e6e92cull, /* j = 13 */
	0x94f4efa8fef70961ull, /* j = 14 */
	0x96942d3720185a00ull, /* j = 15 */
	0x9837f0518db8a96full, /* j = 16 */
	0x99e0459320b7fa65ull, /* j = 17 */
	0x9b8d39b9d54e5539ull, /* j = 18 */
	0x9d3ed9a72cffb751ull, /* j = 19 */
	0x9ef5326091a111aeull, /* j = 20 */
	0xa0b0510fb9714fc2ull, /* j = 21 */
	0xa27043030c496819ull, /* j = 22 */
	0xa43515ae09e6809eull, /* j = 23 */
	0xa5fed6a9b15138eaull, /* j = 24 */
	0xa7cd93b4e965356aull, /* j = 25 */
	0xa9a15ab4ea7c0ef8ull, /* j = 26 */
	0xab7a39b5a93ed337ull, /* j = 27 */
	0xad583eea42a14ac6ull, /* j = 28 */
	0xaf3b78ad690a4375ull, /* j = 29 */
	0xb123f581d2ac2590ull, /* j = 30 */
	0xb311c412a9112489ull, /* j = 31 */
	0xb504f333f9de6484ull, /* j = 32 */
	0xb6fd91e328d17791ull, /* j = 33 */
	0xb8fbaf4762fb9ee9ull, /* j = 34 */
	0xbaff5ab2133e45fbull, /* j = 35 */
	0xbd08a39f580c36bfull, /* j = 36 */
	0xbf1799b67a731083ull, /* j = 37 */
	0xc12c4cca66709456ull, /* j = 38 */
	0xc346ccda24976407ull, /* j = 39 */
	0xc5672a115506daddull, /* j = 40 */
	0xc78d74c8abb9b15dull, /* j = 41 */
	0xc9b9bd866e2f27a3ull, /* j = 42 */
	0xcbec14fef2727c5dull, /* j = 43 */
	0xce248c151f8480e4ull, /* j = 44 */
	0xd06333daef2b2595ull, /* j = 45 */
	0xd2a81d91f12ae45aull, /* j = 46 */
	0xd4f35aabcfedfa1full, /* j = 47 */
	0xd744fccad69d6af4ull, /* j = 48 */
	0xd99d15c278afd7b6ull, /* j = 49 */
	0xdbfbb797daf23755ull, /* j = 50 */
	0xde60f4825e0e9124ull, /* j = 51 */
	0xe0ccdeec2a94e111ull, /* j = 52 */
	0xe33f8972be8a5a51ull, /* j = 53 */
	0xe5b906e77c8348a8ull, /* j = 54 */
	0xe8396a503c4bdc68ull, /* j = 55 */
	0xeac0c6e7dd24392full, /* j = 56 */
	0xed4f301ed9942b84ull, /* j = 57 */
	0xefe4b99bdcdaf5cbull, /* j = 58 */
	0xf281773c59ffb13aull, /* j = 59 */
	0xf5257d152486cc2cull, /* j = 60 */
	0xf7d0df730ad13bb9ull, /* j = 61 */
	0xfa83b2db722a033aull, /* j = 62 */
	0xfd3e0c0cf486c175ull, /* j = 63 */
};
