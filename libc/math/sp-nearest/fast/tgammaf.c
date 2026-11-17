/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * tgammaf.c
 *
 * Created on: Nov 17, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include <math.h>
#include <stdint.h>

#include "internal/math-fast.h"

/*
 * Gamma on [1, 8) in 32 pieces (eight of width 1/8 on [1, 2), eight of 1/4 on [2, 4), sixteen of 1/4 on [4, 8), the
 * double file's layout): the centre m, then a polynomial in t = y - m of degree 6 fitted here (Chebyshev, 160-bit
 * arithmetic; within 2^-29.19 of Gamma relatively, the coefficients' rounding to float included), c0 and c1 as pairs,
 * then c2..c6
 */
static const float pieces[32][10] = {
	{ 0x1.100000p+0f, 0x1.ef66a8p-1f, -0x1.ab3046p-27f, -0x1.da72f2p-2f, 0x1.60d7d4p-27f, 0x1.adf05ap-1f, -0x1.645e4ap-1f, 0x1.7372e2p-1f, -0x1.5df7f0p-1f, 0x1.4e4380p-1f },
	{ 0x1.300000p+0f, 0x1.d7d9d4p-1f, 0x1.3e7bb4p-28f, -0x1.1fd3e8p-2f, -0x1.d46d56p-27f, 0x1.455f6cp-1f, -0x1.ac86c2p-2f, 0x1.ad5100p-2f, -0x1.61f3c2p-2f, 0x1.31e636p-2f },
	{ 0x1.500000p+0f, 0x1.ca931ep-1f, 0x1.51b9c6p-28f, -0x1.1c828cp-3f, -0x1.953ab8p-28f, 0x1.063196p-1f, -0x1.02e1b6p-2f, 0x1.0a1a5ep-2f, -0x1.7ba4aep-3f, 0x1.2f5e4cp-3f },
	{ 0x1.700000p+0f, 0x1.c58ebcp-1f, 0x1.3e0a16p-26f, -0x1.56b928p-6f, 0x1.897094p-31f, 0x1.c10ca8p-2f, -0x1.2ca116p-3f, 0x1.619b54p-3f, -0x1.a76e62p-4f, 0x1.426b5ep-4f },
	{ 0x1.900000p+0f, 0x1.c799aep-1f, 0x1.9a8f2ap-26f, 0x1.543f82p-4f, 0x1.6e2db6p-31f, 0x1.977584p-2f, -0x1.2eaba4p-4f, 0x1.fa1414p-4f, -0x1.df702ap-5f, 0x1.6d79a0p-5f },
	{ 0x1.b00000p+0f, 0x1.d00a6ap-1f, 0x1.aade76p-26f, 0x1.709b26p-3f, -0x1.467f50p-29f, 0x1.85edc4p-2f, -0x1.42c28ep-6f, 0x1.88ebacp-4f, -0x1.08ebe2p-5f, 0x1.bacbe4p-6f },
	{ 0x1.d00000p+0f, 0x1.de9908p-1f, 0x1.1fd384p-26f, 0x1.19924ap-2f, 0x1.35c5c0p-33f, 0x1.87055ap-2f, 0x1.8c05bap-6f, 0x1.4cd07ap-4f, -0x1.04f7cep-6f, 0x1.218204p-6f },
	{ 0x1.f00000p+0f, 0x1.f348f4p-1f, -0x1.04e1bcp-26f, 0x1.7d1ec4p-2f, 0x1.5775e4p-28f, 0x1.97d8d8p-2f, 0x1.01d5f6p-4f, 0x1.334086p-4f, -0x1.36c7e2p-8f, 0x1.9f36eep-7f },
	{ 0x1.100000p+1f, 0x1.0f38cep+0f, 0x1.1cf498p-26f, 0x1.0f6fd4p-1f, -0x1.409ecep-29f, 0x1.cc3678p-2f, 0x1.e7bacap-4f, 0x1.397b92p-4f, 0x1.e0be48p-8f, 0x1.36dc90p-7f },
	{ 0x1.300000p+1f, 0x1.38e5c8p+0f, -0x1.bf44c2p-26f, 0x1.9078b8p-1f, 0x1.17e9f8p-26f, 0x1.2363f2p-1f, 0x1.9fe460p-3f, 0x1.823266p-4f, 0x1.589ea6p-6f, 0x1.321290p-7f },
	{ 0x1.500000p+1f, 0x1.74f994p+0f, -0x1.d758d8p-25f, 0x1.1c757ep+0f, -0x1.0e8f88p-26f, 0x1.8571fcp-1f, 0x1.4115ccp-2f, 0x1.0a26f4p-3f, 0x1.2db77cp-5f, 0x1.8e4704p-7f },
	{ 0x1.700000p+1f, 0x1.c9a76cp+0f, -0x1.a88edep-28f, 0x1.8f2754p+0f, 0x1.cacbeap-25f, 0x1.0d1192p+0f, 0x1.e1f3f0p-2f, 0x1.82b33ap-3f, 0x1.e2e56ap-5f, 0x1.248d76p-6f },
	{ 0x1.900000p+1f, 0x1.202c5cp+1f, -0x1.a89e0ep-24f, 0x1.17cfd0p+1f, -0x1.f43556p-26f, 0x1.7c34dap+0f, 0x1.67a896p-1f, 0x1.207858p-2f, 0x1.7a64d8p-4f, 0x1.c279ecp-6f },
	{ 0x1.b00000p+1f, 0x1.7390dcp+1f, 0x1.f66f2ep-24f, 0x1.8a3a92p+1f, -0x1.38e140p-24f, 0x1.112186p+1f, 0x1.0d29a4p+0f, 0x1.b5401cp-2f, 0x1.27ee34p-3f, 0x1.620a58p-5f },
	{ 0x1.d00000p+1f, 0x1.e98792p+1f, -0x1.eaa49cp-24f, 0x1.17eb80p+2f, -0x1.63b95ep-23f, 0x1.8dcd8cp+1f, 0x1.956f20p+0f, 0x1.4f3476p-1f, 0x1.d0d59cp-3f, 0x1.198b0cp-4f },
	{ 0x1.f00000p+1f, 0x1.48f056p+2f, -0x1.cc49acp-24f, 0x1.914e20p+2f, 0x1.8fbacap-28f, 0x1.252e76p+2f, 0x1.33bc54p+1f, 0x1.037564p+0f, 0x1.6f640cp-2f, 0x1.c3b862p-4f },
	{ 0x1.080000p+2f, 0x1.c2454ep+2f, 0x1.d11212p-24f, 0x1.22a572p+3f, -0x1.bf889ap-22f, 0x1.b4f132p+2f, 0x1.d715f0p+1f, 0x1.953250p+0f, 0x1.246e1ap-1f, 0x1.6d2a0cp-3f },
	{ 0x1.180000p+2f, 0x1.39823ap+3f, 0x1.cfdb9cp-23f, 0x1.a985a2p+3f, -0x1.705b2ap-26f, 0x1.4902eep+3f, 0x1.6babc0p+2f, 0x1.3f0bdep+1f, 0x1.d4ebf0p-1f, 0x1.295378p-2f },
	{ 0x1.280000p+2f, 0x1.bba2dcp+3f, -0x1.1ca52cp-22f, 0x1.3ade5ep+4f, -0x1.e09deap-22f, 0x1.f47806p+3f, 0x1.1b299ep+3f, 0x1.fa7f1ap+1f, 0x1.7ab632p+0f, 0x1.e790d0p-2f },
	{ 0x1.380000p+2f, 0x1.3ea8d2p+4f, 0x1.c10c50p-21f, 0x1.d6ffc4p+4f, 0x1.d6898ep-22f, 0x1.80588ap+4f, 0x1.bcb57ap+3f, 0x1.9537e4p+2f, 0x1.3411f8p+1f, 0x1.927f56p-1f },
	{ 0x1.480000p+2f, 0x1.d05778p+4f, 0x1.cfcd50p-21f, 0x1.640346p+5f, 0x1.038490p-20f, 0x1.29f5bap+5f, 0x1.602374p+4f, 0x1.46b36cp+3f, 0x1.f8ca76p+1f, 0x1.4e80bcp+0f },
	{ 0x1.580000p+2f, 0x1.56e670p+5f, -0x1.129f36p-23f, 0x1.0fe55cp+6f, -0x1.7ab69cp-22f, 0x1.d23c9cp+5f, 0x1.192288p+5f, 0x1.09656ep+4f, 0x1.a0790ap+2f, 0x1.17d4a2p+1f },
	{ 0x1.680000p+2f, 0x1.007a26p+6f, 0x1.bb7082p-19f, 0x1.a38578p+6f, -0x1.7d8c0cp-20f, 0x1.700cfcp+6f, 0x1.c485ecp+5f, 0x1.b2664ap+4f, 0x1.59fbc4p+3f, 0x1.d73ad2p+1f },
	{ 0x1.780000p+2f, 0x1.845dc2p+6f, -0x1.fcb8fep-19f, 0x1.46d8f6p+7f, 0x1.ec1352p-21f, 0x1.2515ecp+7f, 0x1.6f148cp+6f, 0x1.661b6cp+5f, 0x1.2160f8p+4f, 0x1.8f4e94p+2f },
	{ 0x1.880000p+2f, 0x1.29780ap+7f, -0x1.16e078p-18f, 0x1.011790p+8f, 0x1.8588ccp-17f, 0x1.d6c3a8p+7f, 0x1.2c1400p+7f, 0x1.2953cep+6f, 0x1.e74dc2p+4f, 0x1.547d16p+3f },
	{ 0x1.980000p+2f, 0x1.ccc5a6p+7f, 0x1.a3be84p-19f, 0x1.983902p+8f, -0x1.ca97cep-18f, 0x1.7d3a10p+8f, 0x1.ee5550p+7f, 0x1.f1318ep+6f, 0x1.9cfe76p+5f, 0x1.24211ep+4f },
	{ 0x1.a80000p+2f, 0x1.68abc6p+8f, 0x1.4f9638p-17f, 0x1.47091ep+9f, -0x1.f4eff4p-16f, 0x1.3739d0p+9f, 0x1.9a3136p+8f, 0x1.a29168p+7f, 0x1.6045aep+6f, 0x1.f8533ep+4f },
	{ 0x1.b80000p+2f, 0x1.1d34dap+9f, -0x1.0b2fb4p-17f, 0x1.084d30p+10f, 0x1.b33682p-15f, 0x1.001738p+10f, 0x1.56d866p+9f, 0x1.62c148p+8f, 0x1.2e616ap+7f, 0x1.b5ee30p+5f },
	{ 0x1.c80000p+2f, 0x1.c77fcep+9f, 0x1.e9f090p-17f, 0x1.aedb16p+10f, 0x1.d6db16p-15f, 0x1.a8b3b0p+10f, 0x1.2097a0p+10f, 0x1.2ea92ap+9f, 0x1.052b34p+8f, 0x1.7e8336p+6f },
	{ 0x1.d80000p+2f, 0x1.6f2d80p+10f, 0x1.373de8p-15f, 0x1.6219c8p+11f, -0x1.334dd2p-15f, 0x1.62d164p+11f, 0x1.e93a40p+10f, 0x1.03e468p+10f, 0x1.c5e6f8p+8f, 0x1.500a02p+7f },
	{ 0x1.e80000p+2f, 0x1.2aae40p+11f, 0x1.05e866p-14f, 0x1.255e48p+12f, 0x1.01b73ap-13f, 0x1.2a9d04p+12f, 0x1.a17efcp+11f, 0x1.c12cb8p+10f, 0x1.8ccc60p+9f, 0x1.28e3e2p+8f },
	{ 0x1.f80000p+2f, 0x1.ea32d6p+11f, 0x1.e98c04p-16f, 0x1.e9eb48p+12f, -0x1.74c73ep-13f, 0x1.fa3b36p+12f, 0x1.66a796p+12f, 0x1.869432p+11f, 0x1.5cebd6p+10f, 0x1.07c4b4p+9f },
};

/* (n - 1)! for n = 1..13, exact in float */
static const float factorials[13] = { 0x1.000000p+0f, 0x1.000000p+0f, 0x1.000000p+1f, 0x1.800000p+2f, 0x1.800000p+4f, 0x1.e00000p+6f, 0x1.680000p+9f, 0x1.3b0000p+12f, 0x1.3b0000p+15f, 0x1.626000p+18f, 0x1.baf800p+21f, 0x1.308a80p+25f, 0x1.c8cfc0p+28f };

/* An exact sum of two floats, either order */
static inline float tgammaf_sum(float a, float b, float *lo)
{
	float s = a + b;
	float bb = s - a;

	*lo = (a - (s - bb)) + (b - bb);
	return s;
}

/* The piece of y in [1, 8): three fraction bits on [1, 4), four on [4, 8) */
static inline const float *tgammaf_piece(float y)
{
	uint32_t bits = __libc_float_bits(y);

	if (bits < 0x40800000u)
		return pieces[(bits - 0x3f800000u) >> 20];

	return pieces[16 + ((bits - 0x40800000u) >> 19)];
}

/* Gamma at t from its piece's centre as hi + *lo: c0 + c1 t exactly by two fused multiply-adds, the rest in float */
static inline float tgammaf_eval(const float *c, float t, float *lo)
{
	float q = c[4] + t * (c[5] + t * (c[6] + t * (c[7] + t * (c[8] + t * c[9]))));
	float r = __builtin_fmaf(c[3], t, c[1]);

	*lo = __builtin_fmaf(c[3], t, c[1] - r) + (c[2] + t * q);
	return r;
}

/* (n + n_lo)/(d + d_lo) rounded once: the quotient, its remainder by a fused multiply-add, the low parts to first order */
static inline float tgammaf_divide(float n, float n_lo, float d, float d_lo)
{
	float q = n / d;

	return q + ((__builtin_fmaf(-q, d, n) + n_lo) - q * d_lo) / d;
}

/* ln a as hi + *lo for a positive normal a (and a_lo below an ulp of it): log2 to about 2^-35 (__libc_fast_log2f_extended), times ln 2 as a pair */
static inline float tgammaf_log(float a, float a_lo, float *lo)
{
	float l2_lo;
	float l2 = __libc_fast_log2f_extended(a, &l2_lo);
	float l = l2 * 0x1.62e430p-1f;

	*lo = __builtin_fmaf(l2, 0x1.62e430p-1f, -l) + (l2 * -0x1.05c610p-29f + l2_lo * 0x1.62e430p-1f) + a_lo / a;
	return l;
}

/*
 * Stirling's exponent E = (a - 1/2) log a - a + log(2 pi)/2 + S(1/a^2)/a for 8 <= a < 2^24 as hi + *lo: a - 1/2 exact,
 * log a as a pair (2^-35 relatively), their product exact, S of degree 2 (lgammaf's); about 2^-28 absolutely to 50
 */
static inline float tgammaf_stirling(float a, float *lo)
{
	float y = a - 0.5f;
	float w = 1.0f / a;
	float z = w * w;
	float l_lo;
	float l = tgammaf_log(a, 0.0f, &l_lo);
	float p = y * l;
	float p_lo = __builtin_fmaf(y, l, -p) + y * l_lo;
	float s_lo;
	float s = tgammaf_sum(p, -a, &s_lo);
	float t = s + 0x1.d67f1cp-1f;

	*lo = ((s - t) + 0x1.d67f1cp-1f) + (s_lo + p_lo + (0x1.0c97d6p-26f + w * (0x1.555556p-4f + z * (-0x1.6c1418p-9f + z * 0x1.98f67cp-11f))));
	return t;
}

/* sign e^(hi + lo): k = round(hi/ln 2), u = hi - k ln 2 by a fused multiply-add, the float exp kernel and its scaling (overflow and the subnormal results rounded once) */
static inline float tgammaf_exp(float hi, float lo, float sign)
{
	float kf = __builtin_fmaf(hi, 0x1.715476p+0f, 0x1.8p23f) - 0x1.8p23f;
	float u_hi = __builtin_fmaf(-kf, 0x1.62e430p-1f, hi);

	/* Below 2^-151 the result rounds to +-0 (the kernel scales from 2^-153 only) */
	if (kf < -151.0f)
		return sign * 0x1p-100f * 0x1p-100f;

	return sign * __libc_fast_expf_kernel(u_hi, lo - kf * -0x1.05c610p-29f, (int)kf);
}

/* floor(a) for a positive finite a, by masking its fraction bits: exact in every rounding direction */
static inline float tgammaf_floor(float a)
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
 * A negative x, not an integer, from 2^-25 in magnitude, the double file's two methods (D230): above -16 the recurrence (above -8 in the double file: from -8 to -16
 * the float reflection reached 0.97 ulp)
 * Gamma(x) = Gamma(z)/(x (x + 1) ... (x + n + 1)), z = x + n + 2 in (1, 2), the product as a pair, one quotient; from -16
 * the reflection as one exponential of log pi - log(a |sin pi g|) - E(a), sin(pi g) as lgammaf's (a polynomial to 1/4,
 * cos(pi (1/2 - g)) beyond), so that the underflow rounds once; +-0 from -50. The sign is (-1)^(n + 1).
 */
[[gnu::noinline]] static float tgammaf_negative(float x)
{
	float a = -x;
	float n = tgammaf_floor(a);
	float fraction = a - n;
	float sign;

	if (fraction == 0)
		return (x - x) / (x - x);
	sign = tgammaf_floor(n * 0.5f) == n * 0.5f ? -1.0f : 1.0f;
	if (a >= 50.0f)
		return sign * 0x1p-100f * 0x1p-100f;
	if (a < 16.0f) {
		float m = n + 2.0f;
		const float *c = tgammaf_piece(x + m);
		float p_lo = 0.0f;
		float p = x;
		float g_lo;
		float g = tgammaf_eval(c, x - (c[0] - m), &g_lo);

		for (float k = 1.0f; k < m; k += 1.0f) {
			float v_lo;
			float v = tgammaf_sum(x, k, &v_lo);
			float q = p * v;
			float q_lo = __builtin_fmaf(p, v, -q) + (p_lo * v + p * v_lo);

			p = q + q_lo;
			p_lo = q_lo - (p - q);
		}
		return tgammaf_divide(g, g_lo, p, p_lo);
	} else {
		float g = fraction <= 0.5f ? fraction : 1.0f - fraction;
		float sine;
		float sine_lo;
		float q;
		float q_lo;
		float l_lo;
		float l;
		float e_lo;
		float e;
		float first_lo;
		float first;
		float s_lo;
		float s;

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
		q = sine + sine_lo;
		sine_lo -= q - sine;
		sine = q;
		q = a * sine;
		q_lo = __builtin_fmaf(a, sine, -q) + a * sine_lo;
		l = tgammaf_log(q, q_lo, &l_lo);
		e = tgammaf_stirling(a, &e_lo);
		first = tgammaf_sum(0x1.250d04p+0f, -l, &first_lo);
		s = tgammaf_sum(first, -e, &s_lo);
		return tgammaf_exp(s, s_lo + first_lo + (0x1.1cf438p-25f - l_lo - e_lo), sign);
	}
}

/*
 * The fast tgammaf of the single-precision FPU cores (PLAN D162, D219, D232), in float with fused multiply-adds, within
 * 1 ulp (docs/research/libm-fast-float-trig.md, section 25): the ends (NaN, +inf, -inf invalid, +-0 +-inf with
 * divide-by-zero, below 2^-25 1/x, the overflow from 35.0401); the factorials to 13 exactly; Gamma(1 + x)/x below 1;
 * the pieces below 8; e^E to the overflow; negative x by tgammaf_negative. C23 7.12.8.4.
 */
float tgammaf(float x)
{
	uint32_t magnitude = __libc_float_bits(x) & 0x7fffffffu;
	const float *c;
	float lo;
	float hi;

	if (magnitude - 0x33000000u >= 0x4b800000u - 0x33000000u || x > 0x1.18521ep+5f) {
		if (isnan(x))
			return x + x;
		if (x == 0)
			return 1.0f / x;
		if (magnitude < 0x33000000u)
			return 1.0f / x;
		if (x < 0)
			return (x - x) / (x - x);
		return x * 0x1p127f * 0x1p127f;
	}
	if (x < 0)
		return tgammaf_negative(x);
	if (x <= 13.0f && (float)(int32_t)x == x)
		return factorials[(int32_t)x - 1];
	if (x < 1.0f) {
		c = tgammaf_piece(1.0f + x);
		hi = tgammaf_eval(c, x - (c[0] - 1.0f), &lo);
		return tgammaf_divide(hi, lo, x, 0.0f);
	}
	if (x < 8.0f) {
		c = tgammaf_piece(x);
		hi = tgammaf_eval(c, x - c[0], &lo);
		return hi + lo;
	}
	hi = tgammaf_stirling(x, &lo);

	return tgammaf_exp(hi, lo, 1.0f);
}
