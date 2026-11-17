/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * lgammaf.c
 *
 * Created on: Nov 17, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include <math.h>
#include <stdint.h>

#include "internal/math-fast.h"

/*
 * h(y) = lgamma(y)/((y - 1)(y - 2)) on [1/2, 32) in 48 pieces of eight a binade (widths 1/16 to 2), indexed by y's
 * exponent and top three fraction bits: the centre m, then a polynomial in t = y - m of degree 5 fitted here (Chebyshev,
 * 160-bit arithmetic; within 2^-31.3 of h relatively, the coefficients' rounding to float included), c0 and c1 as pairs,
 * then c2..c5
 */
static const float pieces[48][9] = {
	{ 0x1.100000p-1f, 0x1.7dc032p-1f, 0x1.2126f2p-28f, -0x1.14f800p-1f, -0x1.11be7ap-27f, 0x1.41ca4cp-1f, -0x1.baf096p-1f, 0x1.4b8ec0p+0f, -0x1.02f40ep+1f },
	{ 0x1.300000p-1f, 0x1.6d9928p-1f, -0x1.58fcb0p-27f, -0x1.e2b8a4p-2f, 0x1.89c62ap-33f, 0x1.f812d8p-2f, -0x1.37376ep-1f, 0x1.a1863cp-1f, -0x1.245b94p+0f },
	{ 0x1.500000p-1f, 0x1.5f6d78p-1f, -0x1.1090bap-26f, -0x1.aa45f6p-2f, 0x1.8da768p-28f, 0x1.946034p-2f, -0x1.c4c436p-2f, 0x1.132c7ap-1f, -0x1.5d4646p-1f },
	{ 0x1.700000p-1f, 0x1.52d850p-1f, 0x1.87204ap-28f, -0x1.7c8b5cp-2f, -0x1.0bb8d0p-28f, 0x1.4ad5cap-2f, -0x1.52db08p-2f, 0x1.7889aap-2f, -0x1.b50b36p-2f },
	{ 0x1.900000p-1f, 0x1.478f78p-1f, 0x1.2f643ap-26f, -0x1.56d312p-2f, 0x1.94f22ap-29f, 0x1.132662p-2f, -0x1.03b47ep-2f, 0x1.09ca2ap-2f, -0x1.1c32f0p-2f },
	{ 0x1.b00000p-1f, 0x1.3d5ad2p-1f, -0x1.67dc64p-31f, -0x1.373c2ap-2f, -0x1.79006ep-27f, 0x1.d017c8p-3f, -0x1.96318ep-3f, 0x1.814a20p-3f, -0x1.7de786p-3f },
	{ 0x1.d00000p-1f, 0x1.340ef8p-1f, 0x1.9f08c2p-27f, -0x1.1c6f56p-2f, -0x1.ad52c0p-29f, 0x1.8c1964p-3f, -0x1.433456p-3f, 0x1.1dac54p-3f, -0x1.07e772p-3f },
	{ 0x1.f00000p-1f, 0x1.2b89bap-1f, 0x1.600624p-26f, -0x1.05714cp-2f, 0x1.8e774ap-27f, 0x1.55983ap-3f, -0x1.051134p-3f, 0x1.b00ee6p-4f, -0x1.75b7dep-4f },
	{ 0x1.100000p+0f, 0x1.1ffbcap-1f, -0x1.e10a2ep-26f, -0x1.d112f2p-3f, -0x1.6c5064p-28f, 0x1.162ea6p-3f, -0x1.845724p-4f, 0x1.261db2p-4f, -0x1.d08238p-5f },
	{ 0x1.300000p+0f, 0x1.127330p-1f, 0x1.7de6b6p-27f, -0x1.939c7ep-3f, 0x1.8a4eaap-28f, 0x1.b24a22p-4f, -0x1.0fc69ep-4f, 0x1.708bbap-5f, -0x1.04b5a4p-5f },
	{ 0x1.500000p+0f, 0x1.069fc6p-1f, 0x1.b84c74p-26f, -0x1.630c70p-3f, -0x1.bf5216p-28f, 0x1.5b5da8p-4f, -0x1.8a1798p-5f, 0x1.e3ca3ap-6f, -0x1.35f04cp-6f },
	{ 0x1.700000p+0f, 0x1.f85342p-2f, -0x1.413aa8p-29f, -0x1.3bd156p-3f, -0x1.24bd26p-29f, 0x1.1b7122p-4f, -0x1.2620d8p-5f, 0x1.49dbe0p-6f, -0x1.8232cep-7f },
	{ 0x1.900000p+0f, 0x1.e5a06ep-2f, 0x1.787668p-28f, -0x1.1b8a2ep-3f, -0x1.43cc8ep-29f, 0x1.d65388p-5f, -0x1.c1c7bcp-6f, 0x1.d052b2p-7f, -0x1.f477d6p-8f },
	{ 0x1.b00000p+0f, 0x1.d4c5c0p-2f, 0x1.8280d2p-29f, -0x1.009230p-3f, -0x1.a4e922p-28f, 0x1.8bc07cp-5f, -0x1.5f0292p-6f, 0x1.4fb4a8p-7f, -0x1.4f39b2p-8f },
	{ 0x1.d00000p+0f, 0x1.c57824p-2f, -0x1.d011e4p-27f, -0x1.d37b9ep-4f, 0x1.918b64p-29f, 0x1.510c7ep-5f, -0x1.16c722p-6f, 0x1.f0c0b4p-8f, -0x1.ce1030p-9f },
	{ 0x1.f00000p+0f, 0x1.b77c8cp-2f, -0x1.1d8b30p-27f, -0x1.ac6492p-4f, -0x1.90ea6cp-29f, 0x1.221334p-5f, -0x1.c19b32p-7f, 0x1.76f31ap-8f, -0x1.4669bap-9f },
	{ 0x1.100000p+1f, 0x1.a4982ap-2f, 0x1.a469b8p-27f, -0x1.7b5f08p-4f, 0x1.2c0826p-30f, 0x1.d7101ep-6f, -0x1.4da1e6p-7f, 0x1.fd46aep-9f, -0x1.948b12p-10f },
	{ 0x1.300000p+1f, 0x1.8e9332p-2f, -0x1.831fcep-29f, -0x1.476d24p-4f, 0x1.4fc3fap-30f, 0x1.6e52e6p-6f, -0x1.d1b10ap-8f, 0x1.3e3aaap-9f, -0x1.c4b0bap-11f },
	{ 0x1.500000p+1f, 0x1.7b6fdep-2f, 0x1.a5b4b2p-28f, -0x1.1e8916p-4f, 0x1.bb2bccp-30f, 0x1.23f13ap-6f, -0x1.50c782p-8f, 0x1.a0cd10p-10f, -0x1.0c6af4p-11f },
	{ 0x1.700000p+1f, 0x1.6a97b2p-2f, 0x1.840c9cp-27f, -0x1.fb3daap-5f, 0x1.bac678p-30f, 0x1.dac1d2p-7f, -0x1.f57b30p-9f, 0x1.1b9ea0p-10f, -0x1.4dc758p-12f },
	{ 0x1.900000p+1f, 0x1.5b9c82p-2f, 0x1.06d0d4p-28f, -0x1.c54446p-5f, -0x1.e536dcp-30f, 0x1.888962p-7f, -0x1.7e85aap-9f, 0x1.8e813ep-11f, -0x1.afc8ecp-13f },
	{ 0x1.b00000p+1f, 0x1.4e2b6ap-2f, 0x1.c9c77ap-27f, -0x1.9852fcp-5f, 0x1.d1fcbap-30f, 0x1.492f5cp-7f, -0x1.29d2f6p-9f, 0x1.1f9e94p-11f, -0x1.20c62cp-13f },
	{ 0x1.d00000p+1f, 0x1.4204a4p-2f, -0x1.7107eap-27f, -0x1.7267f6p-5f, 0x1.86f28cp-31f, 0x1.176eacp-7f, -0x1.d7fa46p-10f, 0x1.a8e1e2p-12f, -0x1.8d79c8p-14f },
	{ 0x1.f00000p+1f, 0x1.36f620p-2f, -0x1.758b6cp-27f, -0x1.520caap-5f, -0x1.053bd0p-34f, 0x1.df6cdep-8f, -0x1.7bb86ep-10f, 0x1.402baap-12f, -0x1.186a2ap-14f },
	{ 0x1.100000p+2f, 0x1.2818a4p-2f, 0x1.ba7e9ap-28f, -0x1.29a1acp-5f, 0x1.d106c8p-30f, 0x1.8371eep-8f, -0x1.18ce36p-10f, 0x1.b1cfdap-13f, -0x1.5ae1aep-15f },
	{ 0x1.300000p+2f, 0x1.16e1f4p-2f, 0x1.c3292ep-30f, -0x1.fe105ep-6f, -0x1.a6ddb2p-32f, 0x1.2b7a92p-8f, -0x1.862cc6p-11f, 0x1.0e2d8ep-13f, -0x1.833452p-16f },
	{ 0x1.500000p+2f, 0x1.080678p-2f, -0x1.b280c2p-28f, -0x1.bb648ep-6f, 0x1.971682p-31f, 0x1.da9556p-9f, -0x1.18e6b8p-11f, 0x1.60af68p-14f, -0x1.ca0954p-17f },
	{ 0x1.700000p+2f, 0x1.f610aap-3f, -0x1.4717ccp-30f, -0x1.86079ap-6f, 0x1.5a598ep-32f, 0x1.7fc108p-9f, -0x1.a06de4p-12f, 0x1.de5dfap-15f, -0x1.1c10d4p-17f },
	{ 0x1.900000p+2f, 0x1.df17a6p-3f, 0x1.d2a5a4p-28f, -0x1.5a838ep-6f, -0x1.7bfcb0p-34f, 0x1.3b9ebcp-9f, -0x1.3c4548p-12f, 0x1.4eef14p-15f, -0x1.6e8738p-18f },
	{ 0x1.b00000p+2f, 0x1.ca9882p-3f, -0x1.19a74ap-28f, -0x1.367786p-6f, -0x1.90f5bcp-31f, 0x1.0758f0p-9f, -0x1.ea663cp-13f, 0x1.e1d78ep-16f, -0x1.e8fa0ap-19f },
	{ 0x1.d00000p+2f, 0x1.b829f2p-3f, -0x1.f30ab0p-28f, -0x1.18344ap-6f, -0x1.bc977cp-34f, 0x1.bcef64p-10f, -0x1.82fa00p-13f, 0x1.62b38ap-16f, -0x1.4fa01ap-19f },
	{ 0x1.f00000p+2f, 0x1.a779b4p-3f, -0x1.de542ep-29f, -0x1.fd0120p-7f, -0x1.05b6c8p-33f, 0x1.7bede8p-10f, -0x1.36152ap-13f, 0x1.0a64fep-16f, -0x1.d84a40p-20f },
	{ 0x1.100000p+3f, 0x1.912accp-3f, -0x1.33be5cp-28f, -0x1.bd263ap-7f, 0x1.021cdcp-33f, 0x1.3106dep-10f, -0x1.c7f27cp-14f, 0x1.6731b2p-17f, -0x1.22f378p-20f },
	{ 0x1.300000p+3f, 0x1.77868cp-3f, -0x1.f50ad6p-29f, -0x1.7a569ep-7f, -0x1.0cf686p-32f, 0x1.d3b8dap-11f, -0x1.3a6e74p-14f, 0x1.bc8cecp-18f, -0x1.430676p-21f },
	{ 0x1.500000p+3f, 0x1.6190bap-3f, -0x1.f4d5b2p-29f, -0x1.467722p-7f, 0x1.1e83eep-32f, 0x1.6fd340p-11f, -0x1.c19898p-15f, 0x1.205d38p-18f, -0x1.7c1628p-22f },
	{ 0x1.700000p+3f, 0x1.4e7f14p-3f, 0x1.8e8ffep-28f, -0x1.1d3f24p-7f, 0x1.4c8850p-32f, 0x1.275d98p-11f, -0x1.4b1324p-15f, 0x1.84ceb0p-19f, -0x1.d4fda0p-23f },
	{ 0x1.900000p+3f, 0x1.3dbf38p-3f, -0x1.4774b0p-30f, -0x1.f7b810p-8f, 0x1.f644e2p-34f, 0x1.e2b77cp-12f, -0x1.f3c878p-16f, 0x1.0eae90p-19f, -0x1.2d085cp-23f },
	{ 0x1.b00000p+3f, 0x1.2ee432p-3f, -0x1.e8fc52p-29f, -0x1.c0c030p-8f, -0x1.47cdeep-35f, 0x1.9058ccp-12f, -0x1.813652p-16f, 0x1.834a96p-20f, -0x1.8f9f4ep-24f },
	{ 0x1.d00000p+3f, 0x1.219b08p-3f, 0x1.8b19d2p-29f, -0x1.92df46p-8f, 0x1.396ea2p-33f, 0x1.504bd0p-12f, -0x1.2e4b56p-16f, 0x1.1b9dd6p-20f, -0x1.10fc44p-24f },
	{ 0x1.f00000p+3f, 0x1.15a348p-3f, -0x1.414be0p-28f, -0x1.6c1ed4p-8f, 0x1.2a96d8p-33f, 0x1.1da514p-12f, -0x1.e1eb6cp-17f, 0x1.a7e4f4p-21f, -0x1.7e5f0ep-25f },
	{ 0x1.100000p+4f, 0x1.05bbb4p-3f, -0x1.2e0e56p-28f, -0x1.3c48d8p-8f, -0x1.17f012p-37f, 0x1.c74a58p-13f, -0x1.5fb4c0p-17f, 0x1.1bca10p-21f, -0x1.d40824p-26f },
	{ 0x1.300000p+4f, 0x1.e72cf6p-4f, 0x1.0d909ep-30f, -0x1.0aa370p-8f, 0x1.8dc34ap-34f, 0x1.59f858p-13f, -0x1.e0bcd6p-18f, 0x1.5c27ecp-22f, -0x1.01a802p-26f },
	{ 0x1.500000p+4f, 0x1.c85558p-4f, 0x1.012dbap-29f, -0x1.c8d630p-9f, -0x1.bc33f6p-34f, 0x1.0de72cp-13f, -0x1.54e38ap-18f, 0x1.c00944p-23f, -0x1.2cd2eep-27f },
	{ 0x1.700000p+4f, 0x1.adbc60p-4f, 0x1.dd6cecp-32f, -0x1.8c8f8cp-9f, 0x1.1456e0p-35f, 0x1.ae52b4p-14f, -0x1.f24b46p-19f, 0x1.2bcd00p-23f, -0x1.7081b6p-28f },
	{ 0x1.900000p+4f, 0x1.9684c2p-4f, -0x1.c26c08p-29f, -0x1.5c13a4p-9f, -0x1.857874p-35f, 0x1.5d4f04p-14f, -0x1.758472p-19f, 0x1.9e8d8ap-24f, -0x1.d5df90p-29f },
	{ 0x1.b00000p+4f, 0x1.820b00p-4f, 0x1.2e3a90p-32f, -0x1.346b4cp-9f, -0x1.971924p-35f, 0x1.1ff060p-14f, -0x1.1e0f12p-19f, 0x1.26aa80p-24f, -0x1.35e79ep-29f },
	{ 0x1.d00000p+4f, 0x1.6fd36ep-4f, 0x1.aa06c0p-29f, -0x1.138378p-9f, -0x1.36acbep-34f, 0x1.e105f2p-15f, -0x1.be5044p-20f, 0x1.acfbe8p-25f, -0x1.a4e0fap-30f },
	{ 0x1.f00000p+4f, 0x1.5f7e92p-4f, -0x1.91bc80p-30f, -0x1.efbdaep-10f, 0x1.3e5d5cp-35f, 0x1.967102p-15f, -0x1.61cc22p-20f, 0x1.3ec78cp-25f, -0x1.251cbcp-30f },
};

/* An exact sum of two floats, either order */
static inline float lgammaf_sum(float a, float b, float *lo)
{
	float s = a + b;
	float bb = s - a;

	*lo = (a - (s - bb)) + (b - bb);
	return s;
}

/* The piece of y in [1/2, 32) */
static inline const float *lgammaf_piece(float y)
{
	return pieces[(__libc_float_bits(y) - 0x3f000000u) >> 20];
}

/* h at t from its piece's centre as hi + *lo: c0 + c1 t exactly by two fused multiply-adds, the rest in float */
static inline float lgammaf_h(const float *c, float t, float *lo)
{
	float q = c[4] + t * (c[5] + t * (c[6] + t * (c[7] + t * c[8])));
	float r = __builtin_fmaf(c[3], t, c[1]);

	*lo = __builtin_fmaf(c[3], t, c[1] - r) + (c[2] + t * q);
	return r;
}

/* (p + p_lo)(h + h_lo) as hi + *lo, p h exactly */
static inline float lgammaf_scaled(float p, float p_lo, float h, float h_lo, float *lo)
{
	float r = p * h;

	*lo = __builtin_fmaf(p, h, -r) + (p * h_lo + p_lo * h);
	return r;
}

/* ln a as hi + *lo for a positive normal a (and a_lo below an ulp of it): log2 to about 2^-35 (__libc_fast_log2f_extended), times ln 2 as a pair */
static inline float lgammaf_log(float a, float a_lo, float *lo)
{
	float l2_lo;
	float l2 = __libc_fast_log2f_extended(a, &l2_lo);
	float l = l2 * 0x1.62e430p-1f;

	*lo = __builtin_fmaf(l2, 0x1.62e430p-1f, -l) + (l2 * -0x1.05c610p-29f + l2_lo * 0x1.62e430p-1f) + a_lo / a;
	return l;
}

/*
 * lgamma(x) for 2^-30 <= x < 2^120 as hi + *lo (PLAN D229; docs/research/libm-fast-float-trig.md, section 24), the
 * double file's paths in float with fused multiply-adds:
 * - below 1/2, x (x - 1) h(1 + x) - log x (x - 1 as an exact pair; |log x| >= 5 |lgamma(1 + x)|);
 * - below 32, (x - 1)(x - 2) h(x), both factors exact and their product an exact pair, +0 at 1 and 2 (tested before h,
 *   so that no inexact is raised);
 * - from 32 (P11.07d3: below it the pieces spare the log, 162.5 -> the figure in D229), Stirling: (x - 1/2)(log x - 1) + log(2 pi)/2 - 1/2 + S(1/x^2)/x, x - 1/2 with its rounding error, S of
 *   degree 2 (2^-28.5 absolutely)
 */
static float lgammaf_positive(float x, float *lo)
{
	const float *c;
	float h_lo;
	float h;
	float p_lo;
	float p;
	float r_lo;
	float r;
	float s_lo;
	float s;
	float l_lo;
	float l;

	if (x < 0.5f) {
		c = lgammaf_piece(1.0f + x);
		h = lgammaf_h(c, x - (c[0] - 1.0f), &h_lo);
		s = lgammaf_sum(x, -1.0f, &s_lo);
		p = x * s;
		p_lo = __builtin_fmaf(x, s, -p) + x * s_lo;
		r = lgammaf_scaled(p, p_lo, h, h_lo, &r_lo);
		l = lgammaf_log(x, 0.0f, &l_lo);
		s = -l + r;
		*lo = ((-l - s) + r) + (r_lo - l_lo);
		return s;
	}
	if (x < 32.0f) {
		/* x - 1 exact; x - 2 exact from 1, a pair below (x has an ulp of 2^-24 there, x - 2 one of 2^-23) */
		float u = x - 1.0f;
		float v_lo;
		float v = lgammaf_sum(x, -2.0f, &v_lo);

		p = u * v;
		if (p == 0) {
			*lo = 0.0f;
			return 0.0f;
		}
		p_lo = __builtin_fmaf(u, v, -p) + u * v_lo;
		c = lgammaf_piece(x);
		h = lgammaf_h(c, x - c[0], &h_lo);
		return lgammaf_scaled(p, p_lo, h, h_lo, lo);
	} else {
		float y = x - 0.5f;
		float y_lo = (x - y) - 0.5f;
		float w = 1.0f / x;
		float z = w * w;

		l = lgammaf_log(x, 0.0f, &l_lo) - 1.0f;
		p = y * l;
		p_lo = __builtin_fmaf(y, l, -p) + (y * l_lo + y_lo * l);
		s = p + 0x1.acfe3ap-2f;
		*lo = ((p - s) + 0x1.acfe3ap-2f) + (p_lo + (-0x1.e6d052p-27f + w * (0x1.555556p-4f + z * (-0x1.6c1418p-9f + z * 0x1.98f67cp-11f))));
		return s;
	}
}

/* floor(a) for a positive finite a, by masking its fraction bits: exact in every rounding direction */
static inline float lgammaf_floor(float a)
{
	uint32_t bits = __libc_float_bits(a);
	int32_t e = (int32_t)(bits >> 23) - 127;

	if (e < 0)
		return 0.0f;
	if (e >= 23)
		return a;

	return __libc_float_from(bits & ~((1u << (23 - e)) - 1));
}

/*
 * A negative x from 2^-30 in magnitude, a = -x, the double file's two methods (D225): in (-16, -1) the recurrence
 * lgamma(z) - log P, z = x + n + 2 in (1, 2), P = a (a - 1) ... (a - n)(n + 1 - a) as an exact pair product (near
 * every zero both terms are small: 2^-28 absolutely, D219); elsewhere the reflection log pi - log(a |sin pi g|) -
 * lgamma(a), g the distance from a to the nearest integer and sin(pi g) by a polynomial in g to 1/4 (pi as a pair, Q of
 * degree 5 fitted here), by cos(pi (1/2 - g)) beyond, where the sine's polynomial rounded its large correction to
 * 2^-25 (1.4 ulp near -1/2) The poles (g = 0) are +inf with divide-by-zero.
 */
[[gnu::noinline]] static float lgammaf_negative(float x)
{
	float a = -x;
	float n = lgammaf_floor(a);
	float fraction = a - n;
	float g = fraction <= 0.5f ? fraction : 1.0f - fraction;
	float l_lo;
	float l;
	float r_lo;
	float r;
	float s_lo;
	float s;

	if (g == 0)
		return 1.0f / (x - x);
	if (a > 1.0f && a < 16.0f) {
		float p = a;
		float p_lo = 0.0f;

		for (float k = 1.0f; k <= n + 1.0f; k += 1.0f) {
			float v = k <= n ? a - k : k - a;
			float q = p * v;
			float q_lo = __builtin_fmaf(p, v, -q) + p_lo * v;

			p = q + q_lo;
			p_lo = q_lo - (p - q);
		}
		l = lgammaf_log(p, p_lo, &l_lo);
		r = lgammaf_positive(n + 2.0f - a, &r_lo);
		s = lgammaf_sum(r, -l, &s_lo);
		return s + (s_lo + (r_lo - l_lo));
	} else {
		float sine;
		float sine_lo;
		float q;
		float q_lo;

		if (g <= 0.25f) {
			/* sin(pi g) = pi g + g^3 Q(g^2), pi g an exact pair; the rest below 0.1 of it */
			float g2 = g * g;
			float t = g * 0x1.921fb6p+1f;
			float rest = g * (g2 * (-0x1.4abbcep+2f + g2 * (0x1.466bc6p+1f + g2 * (-0x1.32d2ccp-1f + g2 * (0x1.507798p-4f + g2 * (-0x1.e2bb7ap-8f + g2 * 0x1.d8024cp-12f))))));

			sine = t + rest;
			sine_lo = ((t - sine) + rest) + (__builtin_fmaf(g, 0x1.921fb6p+1f, -t) + g * -0x1.777a5cp-24f);
		} else {
			/* sin(pi g) = cos(pi d), d = 1/2 - g exact: 1 + c1 d^2 + d^4 C(d^2), c1 d^2 an exact pair (c1 = -pi^2/2 as a pair), C of degree 3 fitted here (2^-30.5) */
			float d = 0.5f - g;
			float d2 = d * d;
			float d2_lo = __builtin_fmaf(d, d, -d2);
			float w = d2 * -0x1.3bd3ccp+2f;
			float w_lo = __builtin_fmaf(d2, -0x1.3bd3ccp+2f, -w) + (d2_lo * -0x1.3bd3ccp+2f + d2 * -0x1.37c8bcp-23f) + d2 * d2 * (0x1.03c1f0p+2f + d2 * (-0x1.55d3c6p+0f + d2 * (0x1.e1f01ep-3f + d2 * -0x1.a2e2fep-6f)));

			sine = 1.0f + w;
			sine_lo = ((1.0f - sine) + w) + w_lo;
		}
		/* Renormalized: the log takes the low part to first order only (it held the d^4 terms, up to 2^-7, which cost 2^-15) */
		q = sine + sine_lo;
		sine_lo -= q - sine;
		sine = q;
		q = a * sine;
		q_lo = __builtin_fmaf(a, sine, -q) + a * sine_lo;
		float first_lo;
		float first;

		l = lgammaf_log(q, q_lo, &l_lo);
		r = lgammaf_positive(a, &r_lo);
		first = lgammaf_sum(0x1.250d04p+0f, -l, &first_lo);
		s = lgammaf_sum(first, -r, &s_lo);
		return s + (s_lo + first_lo + (0x1.1cf438p-25f - l_lo - r_lo));
	}
}

/*
 * The ends (C23 7.12.8.3, F.10.5.3): NaN returns itself, +-inf +inf, +0 and -0 +inf with divide-by-zero; below 2^-30 in
 * magnitude -log|x|; from 2^120, x (log x - 1) (the other terms are below 2^-110 of it), overflowing past 4.08e36
 */
[[gnu::noinline]] static float lgammaf_ends(float x)
{
	float l_lo;
	float l;

	if (isnan(x))
		return x + x;
	if (isinf(x))
		return __builtin_fabsf(x);
	if (x == 0)
		return 1.0f / __builtin_fabsf(x);
	if (__builtin_fabsf(x) < 0x1p-30f)
		return -logf(__builtin_fabsf(x));
	if (x < 0)
		return lgammaf_negative(x);
	l = lgammaf_log(x, 0.0f, &l_lo);

	return __builtin_fmaf(x, l - 1.0f, x * l_lo);
}

/*
 * The fast lgammaf of the single-precision FPU cores (PLAN D162, D219, D229), in float with fused multiply-adds, within
 * 1 ulp, and near the zeros within 2^-28 absolutely: one test of |x|'s bits sends 0, below 2^-30, from 2^120, the
 * infinities and NaN to lgammaf_ends, a negative x to lgammaf_negative, the rest to lgammaf_positive. C23 7.12.8.3.
 */
float lgammaf(float x)
{
	uint32_t magnitude = __libc_float_bits(x) & 0x7fffffffu;
	float lo;
	float hi;

	if (magnitude - 0x30800000u >= 0x7b800000u - 0x30800000u)
		return lgammaf_ends(x);
	if (x < 0)
		return lgammaf_negative(x);
	hi = lgammaf_positive(x, &lo);

	return hi + lo;
}
