/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * __libc_fast_exp_pairs.c
 *
 * Created on: Nov 12, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include "internal/math-fast.h"

/*
 * For j = 0..63: the bits of T, 2^(j/64) correctly rounded, then the bits of (2^(j/64) - T)/T, computed with 256-bit
 * arithmetic (docs/research/libm-fast-double.md, section 6); side by side, so that one base address serves both
 */
const uint64_t __libc_fast_exp_pairs[128] = {
	0x3ff0000000000000ull, 0x0000000000000000ull, /* j = 0 */
	0x3ff02c9a3e778061ull, 0xbc7160139cd8dc5dull, /* j = 1 */
	0x3ff059b0d3158574ull, 0x3c8cd2523567f613ull, /* j = 2 */
	0x3ff0874518759bc8ull, 0x3c60f74e61e6c861ull, /* j = 3 */
	0x3ff0b5586cf9890full, 0x3c979aa65d837b6dull, /* j = 4 */
	0x3ff0e3ec32d3d1a2ull, 0x3c3ebe3d702f9cd1ull, /* j = 5 */
	0x3ff11301d0125b51ull, 0xbc9556522a2fbd0eull, /* j = 6 */
	0x3ff1429aaea92de0ull, 0xbc91c923b9d5f416ull, /* j = 7 */
	0x3ff172b83c7d517bull, 0xbc801b15eaa59348ull, /* j = 8 */
	0x3ff1a35beb6fcb75ull, 0x3c8b898c3f1353bfull, /* j = 9 */
	0x3ff1d4873168b9aaull, 0x3c9aecf73e3a2f60ull, /* j = 10 */
	0x3ff2063b88628cd6ull, 0x3c8a6f4144a6c38dull, /* j = 11 */
	0x3ff2387a6e756238ull, 0x3c968efde3a8a894ull, /* j = 12 */
	0x3ff26b4565e27cddull, 0x3c80472b981fe7f2ull, /* j = 13 */
	0x3ff29e9df51fdee1ull, 0x3c82f7e16d09ab31ull, /* j = 14 */
	0x3ff2d285a6e4030bull, 0x3c8b3782720c0ab4ull, /* j = 15 */
	0x3ff306fe0a31b715ull, 0x3c834d754db0abb6ull, /* j = 16 */
	0x3ff33c08b26416ffull, 0x3c8fdd395dd3f84aull, /* j = 17 */
	0x3ff371a7373aa9cbull, 0xbc924aedcc4b5068ull, /* j = 18 */
	0x3ff3a7db34e59ff7ull, 0xbc71d1e83e9436d2ull, /* j = 19 */
	0x3ff3dea64c123422ull, 0x3c859f48a72a4c6dull, /* j = 20 */
	0x3ff4160a21f72e2aull, 0xbc58a78f4817895bull, /* j = 21 */
	0x3ff44e086061892dull, 0x3c4363ed60c2ac11ull, /* j = 22 */
	0x3ff486a2b5c13cd0ull, 0x3c6ecce1daa10379ull, /* j = 23 */
	0x3ff4bfdad5362a27ull, 0x3c7690cebb7aafb0ull, /* j = 24 */
	0x3ff4f9b2769d2ca7ull, 0xbc8f94340071a38eull, /* j = 25 */
	0x3ff5342b569d4f82ull, 0xbc78dec6bd0f385full, /* j = 26 */
	0x3ff56f4736b527daull, 0x3c93350518fdd78eull, /* j = 27 */
	0x3ff5ab07dd485429ull, 0x3c9063e1e21c5409ull, /* j = 28 */
	0x3ff5e76f15ad2148ull, 0x3c9432e62b64c035ull, /* j = 29 */
	0x3ff6247eb03a5585ull, 0xbc8c33c53bef4da8ull, /* j = 30 */
	0x3ff6623882552225ull, 0xbc93cedd78565858ull, /* j = 31 */
	0x3ff6a09e667f3bcdull, 0xbc93b3efbf5e2228ull, /* j = 32 */
	0x3ff6dfb23c651a2full, 0xbc6367efb86da9eeull, /* j = 33 */
	0x3ff71f75e8ec5f74ull, 0xbc781f647e5a3ecfull, /* j = 34 */
	0x3ff75feb564267c9ull, 0xbc8619321e55e68aull, /* j = 35 */
	0x3ff7a11473eb0187ull, 0xbc7b32dcb94da51dull, /* j = 36 */
	0x3ff7e2f336cf4e62ull, 0x3c65ebe1abd66c55ull, /* j = 37 */
	0x3ff82589994cce13ull, 0xbc9369b6f13b3734ull, /* j = 38 */
	0x3ff868d99b4492edull, 0xbc94d450d872576eull, /* j = 39 */
	0x3ff8ace5422aa0dbull, 0x3c8db72fc1f0eab4ull, /* j = 40 */
	0x3ff8f1ae99157736ull, 0x3c7bf68359f35f44ull, /* j = 41 */
	0x3ff93737b0cdc5e5ull, 0xbc5da9b88b6c1e29ull, /* j = 42 */
	0x3ff97d829fde4e50ull, 0xbc92434322f4f9aaull, /* j = 43 */
	0x3ff9c49182a3f090ull, 0x3c71affc2b91ce27ull, /* j = 44 */
	0x3ffa0c667b5de565ull, 0xbc87c50422622263ull, /* j = 45 */
	0x3ffa5503b23e255dull, 0xbc91bbd1d3bcbb15ull, /* j = 46 */
	0x3ffa9e6b5579fdbfull, 0x3c8469846e735ab3ull, /* j = 47 */
	0x3ffae89f995ad3adull, 0x3c8c1a7792cb3387ull, /* j = 48 */
	0x3ffb33a2b84f15fbull, 0xbc55c3d956dcaebaull, /* j = 49 */
	0x3ffb7f76f2fb5e47ull, 0xbc68d6f438ad9334ull, /* j = 50 */
	0x3ffbcc1e904bc1d2ull, 0x3c74ffd70a5fddcdull, /* j = 51 */
	0x3ffc199bdd85529cull, 0x3c736eae30af0cb3ull, /* j = 52 */
	0x3ffc67f12e57d14bull, 0x3c84e08fd10959acull, /* j = 53 */
	0x3ffcb720dcef9069ull, 0x3c676b2c6c921968ull, /* j = 54 */
	0x3ffd072d4a07897cull, 0xbc8fad5d3ffffa6full, /* j = 55 */
	0x3ffd5818dcfba487ull, 0x3c74a385a63d07a7ull, /* j = 56 */
	0x3ffda9e603db3285ull, 0x3c8e5a50d5c192acull, /* j = 57 */
	0x3ffdfc97337b9b5full, 0xbc82d52107b43e1full, /* j = 58 */
	0x3ffe502ee78b3ff6ull, 0x3c74b604603a88d3ull, /* j = 59 */
	0x3ffea4afa2a490daull, 0xbc8ff7128fd391f0ull, /* j = 60 */
	0x3ffefa1bee615a27ull, 0x3c8ec3bc41aa2008ull, /* j = 61 */
	0x3fff50765b6e4540ull, 0x3c8a64a931d185eeull, /* j = 62 */
	0x3fffa7c1819e90d8ull, 0x3c77893b4d91cd9dull, /* j = 63 */
};
