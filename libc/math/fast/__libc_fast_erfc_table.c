/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * __libc_fast_erfc_table.c
 *
 * Created on: Nov 11, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include "internal/math-fast.h"

/*
 * R(x) = erfc(x) e^(x^2) for the double-FPU cores' erff and erfcf (docs/research/libm-fast-float-trig.md section 12), the
 * Taylor-form coefficients of degree 8 fitted here by Chebyshev interpolation at 60 digits: rows 0 to 4 in t = x - c on
 * [0.5 + i/2, 1 + i/2) with c the centre, relative error 2^-34.9 to 2^-42.4; row 5 x R(x) as a function of w = 1/x^2 in
 * t = w - 0x1.ef4397f2ba2b9p-5 on [1/10.1^2, 1/9], 2^-35.7
 */
const double __libc_fast_erfc_table[6][9] = {
	{ 0x1.038d54ea3d834p-1, -0x1.78cdd5491eb93p-2, 0x1.d90093a78fff0p-3, -0x1.09e78bede40eap-3, 0x1.119300921d9f6p-4, -0x1.0540387748d6bp-5, 0x1.d42705510a74bp-7, -0x1.95599fcc0a597p-8, 0x1.45f89db6f88bdp-9 },
	{ 0x1.78a692138767ap-2, -0x1.abaacdbc9c22fp-3, 0x1.b56f45ecf28ecp-4, -0x1.9b6364eeb0858p-5, 0x1.68a26121ea1cap-6, -0x1.298d157f9cef3p-7, 0x1.d1aa7b00c9d24p-9, -0x1.619d21276a346p-10, 0x1.f90a237b13372p-12 },
	{ 0x1.23cfc2f1dc7e0p-2, -0x1.0c3d5383a5086p-3, 0x1.c8d0cef03779ap-5, -0x1.6cb53416ffa6bp-6, 0x1.13648c93b8ab7p-7, -0x1.8bef8ac02e823p-9, 0x1.106753d9d370cp-10, -0x1.6d3d2e20f26d0p-12, 0x1.d14b394e69ef2p-14 },
	{ 0x1.d94446d627932p-3, -0x1.6a70d2bae9903p-4, 0x1.0615670dfbacep-5, -0x1.6883fb975e778p-7, 0x1.da59579184a1fp-9, -0x1.2bceae974f800p-10, 0x1.6d7355807fc6fp-12, -0x1.b3aed58ae70d2p-14, 0x1.f1b25235a6790p-16 },
	{ 0x1.8c9eb68ff27d7p-3, -0x1.030578131aa6dp-4, 0x1.43b98bac6e817p-6, -0x1.84e9ac4df74c4p-8, 0x1.c2c730ef1e37bp-10, -0x1.f99a40752b580p-12, 0x1.1319b9ce09c18p-13, -0x1.25bd26d72ac48p-15, 0x1.2e99ab4bf24c3p-17 },
	{ 0x1.18d34c378e28dp-1, -0x1.eba5ecda7ad4fp-3, 0x1.268c5da752a29p-2, -0x1.0f22f103def1bp-1, 0x1.44b872e55c43dp+0, -0x1.d2695f0c0fdcap+1, 0x1.81a03a40165e1p+3, -0x1.8dbf9cbd61731p+5, 0x1.98d278a2a06b2p+7 },
};
