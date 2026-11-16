/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * erfc.c
 *
 * Created on: Nov 16, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include <math.h>
#include <stdint.h>

#include "internal/math-fast.h"

/*
 * R(a) = erfc(a) e^(a^2) on [1/2 + j/2, 1 + j/2] for j = 0..6, as a polynomial in t = a - m, m = 3/4 + j/2, of degree 13,
 * fitted here (Chebyshev, 200-bit arithmetic; within 2^-57.24 of R relatively, the coefficients' rounding included): c0
 * and c1 as pairs, then c2..c13
 */
static const double r_pieces[7][16] = {
	{ 0x1.038d54ea3d834p-1, -0x1.d8ab2f2f78c71p-55, -0x1.78cdd551ee51ap-2, 0x1.1489260ab0f2cp-56, 0x1.d90093ae108b0p-3, -0x1.09e77d40e0214p-3, 0x1.1192f5bd775ccp-4, -0x1.054d68296477dp-5, 0x1.d43a7c4dbe650p-7, -0x1.8c97dd32ae0e1p-8, 0x1.3f81a9597de0ap-9, -0x1.ec0d1ccb09b6fp-11, 0x1.6b80da7254f0fp-12, -0x1.02a2c5796f3cap-13, 0x1.6be408e744acbp-15, -0x1.e34f70587b39ap-17 },
	{ 0x1.78a692138767ap-2, 0x1.a9d91073b5101p-62, -0x1.abaacdbfa8b07p-3, 0x1.d4ac18d1c761dp-57, 0x1.b56f45eef7e3fp-4, -0x1.9b635ac624ac6p-5, 0x1.68a25a664836fp-6, -0x1.299636d6c9209p-7, 0x1.d1b69597f0c3dp-9, -0x1.5b8bc9419d840p-10, 0x1.f0fe8a88d3cd7p-12, -0x1.55c08c80d2503p-13, 0x1.c55c924775cc5p-15, -0x1.22f10017ec710p-16, 0x1.71368494f04cfp-18, -0x1.bde45264f2361p-20 },
	{ 0x1.23cfc2f1dc7e0p-2, 0x1.3c1b1cd31543bp-57, -0x1.0c3d538446447p-3, -0x1.e7547f4055e9bp-57, 0x1.c8d0cef0f8107p-5, -0x1.6cb52fe48945cp-6, 0x1.13648a12017fcp-7, -0x1.8bf716a8ec6bep-9, 0x1.106bd5bb7a2eep-10, -0x1.68388845b02b1p-12, 0x1.cb4c7627a9281p-14, -0x1.1b2919f9d4442p-15, 0x1.5269f21f31c3cp-17, -0x1.88f0ad9976924p-19, 0x1.c3515d18ecf45p-21, -0x1.f0bcbed45e35cp-23 },
	{ 0x1.d94446d627932p-3, -0x1.a7c8fc47f8619p-58, -0x1.6a70d2bb37411p-4, 0x1.feafdc44b03a3p-62, 0x1.0615670e25a7ap-5, -0x1.6883f9919a179p-7, 0x1.da595561f8c9ep-9, -0x1.2bd251bb2f799p-10, 0x1.6d7743d0ced6cp-12, -0x1.aed7ebc6d4759p-14, 0x1.ec774509181bap-16, -0x1.117a6a5f5a9a8p-17, 0x1.27a9379f9d083p-19, -0x1.37b3d0ebc0d92p-21, 0x1.4549d09bbbc55p-23, -0x1.47399628bd723p-25 },
	{ 0x1.8c9eb68ff27d7p-3, -0x1.bb4762f87bc03p-57, -0x1.0305781330099p-4, 0x1.101e4400329b9p-59, 0x1.43b98bac83823p-6, -0x1.84e9ab30e6ab3p-8, 0x1.c2c72fd727ba9p-10, -0x1.f99e41ecb0dd4p-12, 0x1.131bb1602174ep-13, -0x1.2312b2588053ep-15, 0x1.2bfb5df4d61e0p-17, -0x1.2da32c584b240p-19, 0x1.2852bb5b7ada2p-21, -0x1.1ccbd4b77374fp-23, 0x1.0f2b20b252eb3p-25, -0x1.f44283867051ep-28 },
	{ 0x1.54a7a08d4bb45p-3, -0x1.69f6cf54fe69fp-61, -0x1.82a8522b868a1p-5, 0x1.b91700950c889p-60, 0x1.a7eddc9ee6425p-7, -0x1.c24b49c47a2c4p-9, 0x1.d085857a18160p-11, -0x1.d25ebba1c4adbp-13, 0x1.c882f022b048ep-15, -0x1.b45d025ef6bddp-17, 0x1.97dd7b2c0b6fep-19, -0x1.753cad42615a6p-21, 0x1.4ebd276370be6p-23, -0x1.26896dce8ab58p-25, 0x1.00f7472e0f6ddp-27, -0x1.b436b65151e5dp-30 },
	{ 0x1.2a2af19c14930p-3, -0x1.fa044de5671b4p-57, -0x1.2aa6503acda11p-5, -0x1.1d429f75700bep-62, 0x1.22f0664f3cbf9p-7, -0x1.1434ae05873abp-9, 0x1.fff032a0df986p-12, -0x1.cfcdea1b1f611p-14, 0x1.9b50d0d2021e5p-16, -0x1.65778aacf12bdp-18, 0x1.30c2fc4e7a197p-20, -0x1.fe3e344fe3890p-23, 0x1.a3bbca0e4ff01p-25, -0x1.5392b50a5f4dep-27, 0x1.109d8f70540dbp-29, -0x1.ab5f530764396p-32 },
};

/*
 * G(w) = R(1/sqrt(w))/sqrt(w), so that R(a) = v G(v^2) with v = 1/a, on w in [1/27.3^2, 1/100], [1/100, 1/36] and
 * [1/36, 1/16] (a from 4), as a polynomial in t = w - m of degree 10 (within 2^-57.68 relatively): m, then c0 and c1 as
 * pairs, then c2..c10
 */
static const double g_pieces[3][14] = {
	{ 0x1.73a5933ec2451p-8, 0x1.200d896241c92p-1, 0x1.bf4784c519026p-55, -0x1.1c0d730ac6dc0p-2, 0x1.927fc762f650ap-62, 0x1.9f9211de7da9bp-2, -0x1.f53433a0efde4p-1, 0x1.a2a8c091bbdadp+1, -0x1.bcf082c77b182p+3, 0x1.1e025a3800020p+6, -0x1.ae272d4a6d56ep+8, 0x1.71845787b5cf5p+11, -0x1.65b0acc7bb8bfp+14, 0x1.7df224706a8d7p+17 },
	{ 0x1.3579be02468adp-6, 0x1.1e35f4d9b2f2fp-1, 0x1.5d4b967a1574fp-55, -0x1.11901fdab1dd3p-2, -0x1.bf71304907a69p-57, 0x1.7b08925e37e55p-2, -0x1.a79d8054c3d2ep-1, 0x1.4140f8d75672cp+1, -0x1.3009584b5f0a9p+3, 0x1.55c48fbb6ba88p+5, -0x1.b9b3610cd3bb8p+7, 0x1.40bc5d4735d92p+10, -0x1.044a6f2d07522p+13, 0x1.c82d384e2eda4p+15 },
	{ 0x1.71c71c71c71c7p-5, 0x1.1abe657445197p-1, 0x1.54d05155f1793p-56, -0x1.fe7fb5e406cb7p-3, 0x1.db81daafeda97p-57, 0x1.41712926f75bep-2, -0x1.3bcef9fcf4107p-1, 0x1.98e51ca27c723p+0, -0x1.41d74843dd559p+2, 0x1.25ed3f0d31febp+4, -0x1.2e207a835f429p+6, 0x1.565c71b0339edp+8, -0x1.afc7cd572df6cp+10, 0x1.1dca83f0799d6p+13 },
};

/*
 * (erf(x)/x - 2/sqrt(pi))/x^2 = p0 + z Q(z) for z = x^2 <= 1/4, p0 = -2/(3 sqrt(pi)) as a pair: the fast erf's series
 * (libc/math/fast/erf.c, x^5 Q within 2^-61.9 of the rest of erf, relatively)
 */
static inline double erfc_series(double z)
{
	return 0x1.ce2f21a042be2p-4 + z * (-0x1.b82ce31288b4bp-6 + z * (0x1.565bcd0e69b0dp-8 + z * (-0x1.c02db3ff6f002p-11 + z * (0x1.f9a32672932d0p-14 + z * (-0x1.f4d22b461bfb6p-17 + z * (0x1.b9dc5927a3862p-20 + z * (-0x1.5e26f32b771b7p-23 + z * 0x1.cf94c6515b7f6p-27)))))));
}

/*
 * erfc below 1/2 as 1 - erf(x): 1 - x 2/sqrt(pi) and then - x^3 p0 by fast two-sums of exact products (x^2, x^3 and
 * both products as pairs, 2/sqrt(pi) and p0 as pairs; |x 2/sqrt(pi)| < 0.57 and the result is at least 0.47), the rest
 * below 2^-7 of the result rounded in plain double; out of line, so that the main path saves no registers
 */
[[gnu::noinline]] static double erfc_small(double x)
{
	struct __libc_dd square = __libc_fast_product(x, x);
	struct __libc_dd cube = __libc_fast_product(x, square.hi);
	struct __libc_dd term = __libc_fast_product(cube.hi, -0x1.812746b0379e7p-2);
	struct __libc_dd product = __libc_fast_product(x, 0x1.20dd750429b6dp+0);
	struct __libc_dd first = __libc_dd_fast_sum(1.0, -product.hi);
	struct __libc_dd second = __libc_dd_fast_sum(first.hi, -term.hi);
	double z = square.hi;

	return second.hi + ((first.lo + second.lo) - (product.lo + x * 0x1.1ae3a914fed80p-56 + term.lo + cube.hi * 0x1.ee12e49cab700p-57 + (cube.lo + x * square.lo) * -0x1.812746b0379e7p-2 + cube.hi * z * erfc_series(z)));
}

/*
 * The ends (C23 7.12.8.2, F.10.5.2, and the strict file): NaN returns itself, +inf +0 and -inf 2; below 2^-56, 1 - x
 * (2/sqrt(pi) x is below a quarter of an ulp of 1, and x itself rounds the same way in every direction, with no
 * spurious underflow from a product); below -6, 2 - tiny (erfc 6 is below 2^-55); above 27.3, the underflow (erfc is
 * below 2^-1075 from 27.23)
 */
[[gnu::noinline]] static double erfc_ends(double x)
{
	volatile double tiny = 0x1p-80;

	if (isnan(x))
		return x + x;
	if (isinf(x))
		return x > 0 ? 0.0 : 2.0;
	if (__builtin_fabs(x) < 0x1p-56)
		return 1.0 - x;
	if (x < 0)
		return 2.0 - tiny;

	return __libc_math_underflow(1.0);
}

/*
 * c0 + c1 t + t^2 Q(t) for a row c0_hi, c0_lo, c1_hi, c1_lo, c2.. (q = c1_lo + t Q(t), already evaluated) as hi + *lo:
 * c0 + c1 t exactly (two fused multiply-adds, or an exact product and a fast two-sum: |c1 t| is below c0), the rest
 * added in plain double
 */
[[gnu::always_inline]] static inline double erfc_piece(const double *c, double t, double q, double *lo)
{
	struct __libc_dd product;
	struct __libc_dd sum;
	double r;

	if (__libc_fpu_fma) {
		r = __builtin_fma(c[2], t, c[0]);
		*lo = __builtin_fma(c[2], t, c[0] - r) + (c[1] + t * q);
		return r;
	}
	product = __libc_fast_product(c[2], t);
	sum = __libc_dd_fast_sum(c[0], product.hi);
	*lo = sum.lo + product.lo + (c[1] + t * q);

	return sum.hi;
}

/*
 * The fast erfc of the double-FPU cores (PLAN D162, D219, D224), within 1 ulp:
 * - below 1/2 in magnitude, 1 - erf(x) by erf's series (erfc_small);
 * - from 1/2 to 27.3, erfc(a) = e^(-a^2) R(a), a = |x|: a^2 as an exact pair z_hi + z_lo, e^(-a^2) by the fast exp's
 *   reduction (k = round(-64 z_hi/ln 2), u = -z_hi - k ln 2/64 - z_lo, both ln 2 parts as pow takes them) and table, as
 *   S (1 + tau) with S = 2^(k/64) and tau = its tail + (e^u - 1); R by seven pieces of degree 13 below 4, and from 4 as
 *   v G(v^2), v = 1/a with its remainder, by three pieces of degree 10; the result S R_hi as an exact product plus
 *   S (R_lo + R_hi tau), rounded once. From 26 (results from 2^-1022 down) S is scaled by 2^200 and the sum by 2^-200,
 *   one more rounding onto the subnormal grid, which raises underflow itself;
 * - for negative x, 2 - erfc(a) by a fast two-sum (the result is at least 1.5); below -6, 2 - tiny.
 */
double erfc(double x)
{
	uint32_t high = (uint32_t)(__libc_double_bits(x) >> 32) & 0x7fffffffu;
	double a = __builtin_fabs(x);
	struct __libc_dd square;
	struct __libc_dd product;
	struct __libc_dd sum;
	const uint64_t *entry;
	const double *c;
	double scale;
	double tau;
	double r_hi;
	double r_lo;
	double lo;
	double kd;
	double q;
	double t;
	double u;
	int32_t bias = 0;
	int32_t k;

	/* 2^-56 <= |x| < 27.3, and x above -6, from the high word */
	if (high - 0x3c700000u >= 0x403b4cccu - 0x3c700000u || (x < 0 && high >= 0x40180000u))
		return erfc_ends(x);
	if (high < 0x3fe00000u)
		return erfc_small(x);

	if (high < 0x40100000u) {
		/* The piece j = floor(2 a) - 1, t = a - m exact (a within a factor of 2 of m = 3/4 + j/2) */
		k = (int32_t)(a * 2.0) - 1;
		c = r_pieces[k];
		t = a - (0.75 + 0.5 * (double)k);
		q = c[3] + t * (c[4] + t * (c[5] + t * (c[6] + t * (c[7] + t * (c[8] + t * (c[9] + t * (c[10] + t * (c[11] + t * (c[12] + t * (c[13] + t * (c[14] + t * c[15])))))))))));
		r_hi = erfc_piece(c, t, q, &r_lo);
	} else {
		double v;
		double v_lo;
		double g_hi;
		double g_lo;
		double w;

		/* v = 1/a + v_lo, the remainder exact; w = v^2 to about 2^-53 */
		v = 1.0 / a;
		product = __libc_fast_product(v, a);
		v_lo = ((1.0 - product.hi) - product.lo) / a;
		w = v * v;
		w += 2.0 * v * v_lo;
		c = g_pieces[w < 0x1.47ae147ae147bp-7 ? 0 : (w < 0x1.c71c71c71c71cp-6 ? 1 : 2)];
		t = w - c[0];
		q = c[4] + t * (c[5] + t * (c[6] + t * (c[7] + t * (c[8] + t * (c[9] + t * (c[10] + t * (c[11] + t * (c[12] + t * c[13]))))))));
		g_hi = erfc_piece(c + 1, t, q, &g_lo);
		product = __libc_fast_product(v, g_hi);
		r_hi = product.hi;
		r_lo = product.lo + (v * g_lo + v_lo * g_hi);
		if (high >= 0x403a0000u)
			bias = 200;
	}

	/* e^(-a^2) = S (1 + tau) */
	square = __libc_fast_product(a, a);
	k = __libc_fast_exp_index(square.hi * -0x1.71547652b82fep+6, &kd);
	u = ((-square.hi - kd * 0x1.62e42fee00000p-7) - kd * 0x1.a39ef35793c76p-39) - square.lo;
	entry = __libc_fast_exp_entry(k);
	scale = __libc_double_from(__libc_fast_exp_scale_bits(entry, (k >> 6) + bias));
	tau = __libc_fast_expm1_tail(u, __libc_double_from(entry[1]));

	product = __libc_fast_product(scale, r_hi);
	/* r_lo holds the pieces' t^2 terms too, up to 2^-6 of R: its product with tau is kept */
	lo = product.lo + scale * (r_lo + (r_hi + r_lo) * tau);
	if (x < 0) {
		sum = __libc_dd_fast_sum(2.0, -product.hi);
		return sum.hi + (sum.lo - lo);
	}
	if (bias != 0)
		return (product.hi + lo) * 0x1p-200;

	return product.hi + lo;
}
