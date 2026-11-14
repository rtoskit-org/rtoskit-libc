/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * __libc_fast_erfcf_r_table.c
 *
 * Created on: Nov 11, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include "internal/math-fast.h"

/*
 * R(x) = erfc(x) e^(x^2) for the single-precision FPU cores' erfcf (docs/research/libm-fast-float-trig.md section 15),
 * degree 9 fitted here: the coefficients of t^0 to t^4 as float pairs (hi, lo), then each of t^5 to t^9 rounded to float
 * in turn and the rest refitted to the residual by relative least squares, at 50 digits. Rows 0 to 5 in t = x - c on
 * [i/2, (i + 1)/2) with c the centre, relative error 2^-37.0 to 2^-47.3; row 6 x R(x) as a function of w = 1/x^2 in
 * t = w - 0x1.f7811ap-5f on [1/9.2^2, 1/9], 2^-39.2
 */
const float __libc_fast_erfcf_r_table[7][15] = {
	{ 0x1.8a6adcp-1f, 0x1.b4736ep-26f, -0x1.7c857cp-1f, 0x1.94355ep-27f, 0x1.2b497ep-1f, -0x1.7cdb1ap-28f, -0x1.97997cp-2f, 0x1.8c52a4p-27f, 0x1.f0ace4p-3f, -0x1.4cf7dap-29f, -0x1.146966p-3f, 0x1.1cf366p-4f, -0x1.132e4ap-5f, 0x1.01b84ep-6f, -0x1.b82814p-8f },
	{ 0x1.038d54p-1f, 0x1.d47ddep-26f, -0x1.78cdd6p-2f, 0x1.5c3daep-27f, 0x1.d90094p-3f, -0x1.8f43ccp-29f, -0x1.09e77ep-3f, 0x1.5b9ae4p-28f, 0x1.119308p-4f, -0x1.3f19dep-30f, -0x1.054d6p-5f, 0x1.d42152p-7f, -0x1.8c93b2p-8f, 0x1.46af3ap-9f, -0x1.f33f84p-11f },
	{ 0x1.78a692p-2f, 0x1.387d98p-30f, -0x1.abaacep-3f, 0x1.01693ep-29f, 0x1.b56f46p-4f, -0x1.69d7a6p-32f, -0x1.9b635ap-5f, -0x1.9b0f1ap-30f, 0x1.68a266p-6f, -0x1.a4652ap-32f, -0x1.299634p-7f, 0x1.d1a6eap-9f, -0x1.5b887ep-10f, 0x1.f9ee6ap-12f, -0x1.59f852p-13f },
	{ 0x1.23cfc2p-2f, 0x1.e3b926p-27f, -0x1.0c3d54p-3f, 0x1.eee8a4p-29f, 0x1.c8d0cep-5f, 0x1.ddc912p-30f, -0x1.6cb53p-6f, 0x1.97fcdp-34f, 0x1.13648ep-7f, 0x1.e33ee4p-34f, -0x1.8bf716p-9f, 0x1.1066p-10f, -0x1.68354ep-12f, 0x1.d1f51ep-14f, -0x1.1e41e6p-15f },
	{ 0x1.d94446p-3f, 0x1.ac4f38p-28f, -0x1.6a70d2p-4f, -0x1.766e36p-29f, 0x1.061568p-5f, -0x1.e49ad8p-30f, -0x1.6883fap-7f, 0x1.b7095p-33f, 0x1.da595ap-9f, -0x1.fd9dbep-34f, -0x1.2bd252p-10f, 0x1.6d722ep-12f, -0x1.aed456p-14f, 0x1.f246a2p-16f, -0x1.14374ap-17f },
	{ 0x1.8c9eb6p-3f, 0x1.1fe5p-28f, -0x1.030578p-4f, -0x1.330026p-32f, 0x1.43b98cp-6f, -0x1.4ee228p-32f, -0x1.84e9acp-8f, 0x1.9dc50ep-33f, 0x1.c2c732p-10f, -0x1.3fd3fcp-36f, -0x1.f99e42p-12f, 0x1.13192p-13f, -0x1.23116ep-15f, 0x1.2ee68p-17f, -0x1.2fe294p-19f },
	{ 0x1.18b3bp-1f, -0x1.50d8e2p-26f, -0x1.ea776p-3f, 0x1.cf383ep-28f, 0x1.24eb76p-2f, -0x1.1656d6p-27f, -0x1.0c89ep-1f, 0x1.df87dp-26f, 0x1.4010fep+0f, -0x1.708538p-25f, -0x1.ca7f66p+1f, 0x1.777f9ep+3f, -0x1.57c168p+5f, 0x1.8b9908p+7f, -0x1.b8e49ep+9f },
};
