/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * __libc_fast_erf_pieces.c
 *
 * Created on: Nov 14, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include "internal/math-fast.h"

/*
 * erf(c + t) ~ sum of a_i t^i for |t| <= 1/4 about c = 1/2, 1, ..., 4 (piece j at c = (j + 1)/2), least squares on
 * Chebyshev nodes with 50-digit arithmetic over [max(c - 1/4, 1/2), min(c + 1/4, 3.92)]: within 2^-30.7 of erf
 * (docs/research/libm-fast-float-trig.md section 22)
 */
const double __libc_fast_erf_pieces[8][8] = {
	{ 0x1.0a7ef5c187d1ap-1, 0x1.c1efca57da6f4p-1, -0x1.c1efd3b63412cp-2, -0x1.2bf2cdf8f420dp-3, 0x1.76cc3d389b016p-3, 0x1.04ed5ff7f6fb1p-7, -0x1.b3c8e537b057dp-5, 0x1.c3a2d90bb16e0p-7 },
	{ 0x1.af767a78c84bap-1, 0x1.a911f0a83bfb4p-2, -0x1.a91203810d7f9p-2, 0x1.1b612e4ba156fp-3, 0x1.1b7906ab3f5f8p-4, -0x1.1b5456ea4ac32p-4, 0x1.0802b901e1283p-8, 0x1.e7f0eeac9f295p-7 },
	{ 0x1.eea55573a6327p-1, 0x1.e723722de6a28p-4, -0x1.6d5aa94346159p-3, 0x1.1c2a1c73ae4a5p-3, -0x1.6d4249ad56335p-5, -0x1.e78036a17d602p-7, 0x1.37c91b00451b0p-6, -0x1.274e40156c74ep-8 },
	{ 0x1.fd9ae140e1f52p-1, 0x1.529b9e9210918p-6, -0x1.529b6b8c86d07p-5, 0x1.8b0ae17664010p-5, -0x1.1a3c53a93e922p-5, 0x1.ace94d6a072bcp-7, 0x1.578bd69de4ea9p-11, -0x1.bb9eff6c18a0bp-9 },
	{ 0x1.ffcaa8f4828e6p-1, 0x1.1d831835fc5bfp-9, -0x1.64e395d0a0da1p-8, 0x1.119d637432083p-7, -0x1.1a94c6289b1e8p-7, 0x1.9103ac703312ep-8, -0x1.6a6bc3b6d12e2p-9, 0x1.09f70170560d5p-11 },
	{ 0x1.fffd1ac4361e4p-1, 0x1.2408e8d2ccdb0p-13, -0x1.b60f8b199afadp-12, 0x1.9db77bb9f4e6dp-11, -0x1.119ccd3ec6769p-10, 0x1.0a75c9d0d959bp-10, -0x1.8bb0075cc54d2p-11, 0x1.9b7161cf3a422p-12 },
	{ 0x1.ffffe710de8c6p-1, 0x1.6a594dc7fbd30p-18, -0x1.3d1767a084e83p-16, 0x1.62d0b2a904ecdp-15, -0x1.1b510057580f3p-14, 0x1.57fecd2623c61p-14, -0x1.5841c78e40aeap-14, 0x1.fa49d54283011p-15 },
	{ 0x1.ffffff7b95899p-1, 0x1.10ef7fa60b436p-23, -0x1.0f3eb14807ae6p-21, 0x1.69cdb5cede208p-20, -0x1.231ff3c9b7188p-19, 0x1.4c2403c40cf40p-18, 0x1.d2857ffa0dd32p-24, 0x1.55ccace12aefbp-17 },
};
