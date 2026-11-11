/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * __libc_fast_erfcf_table.c
 *
 * Created on: Nov 11, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include "internal/math-fast.h"

/*
 * R(x) = erfc(x) e^(x^2) for the single-precision FPU cores' erff and erfcf (docs/research/libm-fast-float-trig.md
 * section 13), degree 7 fitted here: the constant term as a float pair, then each coefficient rounded to float in turn
 * and the rest refitted to the residual by relative least squares. Rows 0 to 4 in t = x - c on [0.5 + i/2, 1 + i/2) with
 * c the centre, relative error 2^-29.6 to 2^-33.9; row 5 x R(x) as a function of w = 1/x^2 in t = w - 0x1.ef4398p-5 on
 * [1/10.1^2, 1/9], 2^-32.2
 */
const float __libc_fast_erfcf_table[6][9] = {
	{ 0x1.038d54p-1f, 0x1.cf7d26p-26f, -0x1.78cdd6p-2f, 0x1.d900a8p-3f, -0x1.09e6fcp-3f, 0x1.118646p-4f, -0x1.057fccp-5f, 0x1.de5686p-7f, -0x1.8485ecp-8f },
	{ 0x1.78a692p-2f, 0x1.28dd3cp-30f, -0x1.abaacep-3f, 0x1.b56f4ep-4f, -0x1.9b62fcp-5f, 0x1.689862p-6f, -0x1.29bb2p-7f, 0x1.d999fep-9f, -0x1.557bc4p-10f },
	{ 0x1.23cfc2p-2f, 0x1.e34578p-27f, -0x1.0c3d54p-3f, 0x1.c8d0d2p-5f, -0x1.6cb3bep-6f, 0x1.13606ap-7f, -0x1.8d30ccp-9f, 0x1.13e716p-10f, -0x1.19d836p-12f },
	{ 0x1.d94446p-3f, 0x1.ac1132p-28f, -0x1.6a70d2p-4f, 0x1.061568p-5f, -0x1.68862ap-7f, 0x1.da548ep-9f, -0x1.29f162p-10f, 0x1.7150b4p-12f, -0x1.55535cp-13f },
	{ 0x1.8c9eb6p-3f, 0x1.1fd21ep-28f, -0x1.030578p-4f, 0x1.43b98cp-6f, -0x1.84ea1ep-8f, 0x1.c2c4fep-10f, -0x1.f8d87ep-12f, 0x1.15366ep-13f, -0x1.57bf4cp-15f },
	{ 0x1.18d34cp-1f, 0x1.b6155cp-28f, -0x1.eba5ecp-3f, 0x1.268c98p-2f, -0x1.0f24dap-1f, 0x1.4449b8p+0f, -0x1.d12cbcp+1f, 0x1.a34228p+3f, -0x1.9d48e4p+5f },
};
