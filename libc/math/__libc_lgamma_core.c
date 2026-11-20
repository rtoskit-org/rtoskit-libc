/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * __libc_lgamma_core.c
 *
 * Created on: Oct 19, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include <math.h>
#include <stdint.h>

#include "internal/math.h"

/* zeta(k) - 1 for k = 2 ... 40 */
static const double zeta_minus_one[] = { 0x1.4a34cc4a60fa6p-1, 0x1.9dd002780310ap-3, 0x1.51322ac7d8483p-4, 0x1.2e831d94f99b7p-5, 0x1.1c26130249124p-6, 0x1.1196d0a679c47p-7, 0x1.0b36af86396e9p-8, 0x1.073e7b02d6ae0p-9, 0x1.04b8ce96ee5f8p-10, 0x1.0318df2459954p-11, 0x1.020a5b2cd3042p-12, 0x1.01593a1177bd6p-13, 0x1.00e4af2b4e156p-14, 0x1.0097bcbf11bedp-15, 0x1.0064cdeb22f0fp-16, 0x1.0043073686681p-17, 0x1.002c9953744ccp-18, 0x1.001db08f9ba4ap-19, 0x1.0013c594466eap-20, 0x1.000d2bab28121p-21, 0x1.0008c66cec77dp-22, 0x1.0005d8f13858cp-23, 0x1.0003e59ffde12p-24, 0x1.000298ea55633p-25, 0x1.0001bb316ccdap-26, 0x1.0001276b90845p-27, 0x1.0000c4ed05ae3p-28, 0x1.0000834601a87p-29, 0x1.00005782aaebep-30, 0x1.00003a56719b9p-31, 0x1.000026e3f644fp-32, 0x1.000019ed24201p-33, 0x1.00001148ad653p-34, 0x1.00000b85be414p-35, 0x1.000007ae797ffp-36, 0x1.0000051ef8ffap-37, 0x1.00000369f9ff9p-38, 0x1.00000246a5ffbp-39, 0x1.000001846e551p-40 };

static struct __libc_dd log_dd(struct __libc_dd a)
{
	struct __libc_dd log = __libc_log_core(a.hi);

	return __libc_dd_fast_sum(log.hi, log.lo + a.lo / a.hi);
}

/*
 * Stirling's series for y >= 12 (a double-double): (y - 1/2) log y - y +
 * log(2 pi)/2 + sum B_2k / (2k (2k - 1) y^(2k - 1)), ten terms (the next is
 * below 2^-66 of the result at 12).
 */
static struct __libc_dd stirling(struct __libc_dd y)
{
	const struct __libc_dd half_log_2pi = { .hi = 0x1.d67f1c864beb5p-1, .lo = -0x1.65b5a1b7ff5dfp-55 };
	double z = 1.0 / y.hi;
	double z2 = z * z;
	double series = z * (0x1.5555555555555p-4 + z2 * (-0x1.6c16c16c16c17p-9 + z2 * (0x1.a01a01a01a01ap-11 + z2 * (-0x1.3813813813814p-11 + z2 * (0x1.b951e2b18ff23p-11 + z2 * (-0x1.f6ab0d9993c7dp-10 + z2 * (0x1.a41a41a41a41ap-8 + z2 * (-0x1.e4286cb0f5398p-6 + z2 * (0x1.6fe96381e0680p-3 + z2 * (-0x1.6476701181f3ap+0))))))))));
	struct __libc_dd value = __libc_dd_multiply(__libc_dd_add(y, (struct __libc_dd){ .hi = -0.5, .lo = 0.0 }), log_dd(y));

	value = __libc_dd_add(value, __libc_dd_negate(y));
	value = __libc_dd_add(value, half_log_2pi);

	return __libc_dd_fast_sum(value.hi, value.lo + series);
}

/*
 * The first terms, as double-doubles (in plain double the third and fourth left 0.60 ulp near 5/4 and 3/4, and
 * the shift to 12 cancelled to 0.68 ulp near the minimum, P12.01):
 * - lgamma(1 + z) = -gamma z + sum_(k >= 2) (-1)^k zeta(k) z^k / k and lgamma(2 + z) = (1 - gamma) z +
 *   sum_(k >= 2) (-1)^k (zeta(k) - 1) z^k / k for |z| <= 1/4, relative at the zeros 1 and 2: k = 1 ... 4;
 * - lgamma(3/2 + t) = sum c_k t^k, c_k = psi^(k - 1)(3/2)/k! (mpmath at 300 bits), for |t| <= 1/4 about the
 *   minimum near 1.4616 (-0.1215): k = 0 ... 4.
 */
static const struct __libc_dd near_one[] = { { -0x1.2788cfc6fb619p-1, 0x1.6cb90701fbfabp-58 }, { 0x1.a51a6625307d3p-1, 0x1.1873d8912200cp-56 }, { -0x1.9a4d55beab2d7p-2, 0x1.4c26d1b465993p-59 }, { 0x1.151322ac7d848p-2, 0x1.b5f91211196e5p-57 } };
static const struct __libc_dd near_two[] = { { 0x1.b0ee6072093cep-2, 0x1.6cb90701fbfabp-58 }, { 0x1.4a34cc4a60fa6p-2, 0x1.1873d8912200cp-56 }, { -0x1.13e001a557607p-4, 0x1.fb68be2f8821fp-58 }, { 0x1.51322ac7d8483p-6, 0x1.afc89088cb729p-60 } };
static const struct __libc_dd near_three_halves[] = { { -0x1.eeb95b094c191p-4, -0x1.346863f58b075p-58 }, { 0x1.2aed059bd608ap-5, 0x1.cd3d2ca77b63ap-63 }, { 0x1.de9e64df22ef3p-2, -0x1.6d48ec9933fbap-57 }, { -0x1.1ae55b180726cp-3, -0x1.959aeebbe37a9p-59 }, { 0x1.e0f840dad61dap-5, -0x1.599fc3fe0a24cp-59 } };

/* The rest about 3/2 in plain double, c5 ... c24 (2^-66 at |t| = 1/4) */
static const double three_halves_tail[] = { -0x1.da59d5374a543p-6, 0x1.f9ca39daa929cp-7, -0x1.1a8ba4f0ea597p-7, 0x1.456f1ad666a3bp-8, -0x1.7edb812f6426ep-9, 0x1.c9735ae9db2c1p-10, -0x1.148a319eec639p-10, 0x1.517c5a1579f10p-11, -0x1.9eff1d1c8bdc2p-12, 0x1.00c41c13e4c1cp-12, -0x1.3f6dff22ac1c2p-13, 0x1.8f3619541742cp-14, -0x1.f4ea079c9c87ap-15, 0x1.3b5e73f18d398p-15, -0x1.8e583480fb843p-16, 0x1.f88eb43555368p-17, -0x1.4059677eed115p-17, 0x1.97b6b03fa7446p-18, -0x1.03fd6bf0808efp-18, 0x1.4c355353d5241p-19 };

/*
 * lgamma(y) for a double-double y in [3/4, 9/4]: within 1/4 of 1 or 2 the zeta series (z = y - 1 or y - 2, the rest
 * to k = 40, zeta(k) - 1 below 2^-(k - 1)), else the series about 3/2; the plain tail, then one double-double Horner
 * over the first terms
 */
static struct __libc_dd lgamma_unit(struct __libc_dd y)
{
	double centre = y.hi <= 1.25 ? 1.0 : (y.hi >= 1.75 ? 2.0 : 1.5);
	struct __libc_dd t = __libc_dd_add(y, (struct __libc_dd){ .hi = -centre, .lo = 0.0 });
	const struct __libc_dd *head = centre == 1.5 ? near_three_halves : (centre == 2.0 ? near_two : near_one);
	int count = centre == 1.5 ? 5 : 4;
	double tail = 0.0;
	struct __libc_dd value;

	if (centre == 1.5) {
		for (int k = (int)(sizeof(three_halves_tail) / sizeof(three_halves_tail[0])) - 1; k >= 0; --k)
			tail = tail * t.hi + three_halves_tail[k];
	} else {
		for (int k = 40; k >= 5; --k) {
			double coefficient = (centre == 2.0 ? zeta_minus_one[k - 2] : 1.0 + zeta_minus_one[k - 2]) / k;

			tail = tail * t.hi + ((k & 1) != 0 ? -coefficient : coefficient);
		}
	}
	value = (struct __libc_dd){ .hi = tail, .lo = 0.0 };
	for (int k = count - 1; k >= 0; --k)
		value = __libc_dd_add(__libc_dd_multiply(value, t), head[k]);

	/* the zeta series have no constant term */
	return centre == 1.5 ? value : __libc_dd_multiply(value, t);
}

/*
 * log gamma(x) for a positive double-double x below 2^60: [3/4, 9/4] directly; below 3/4 lgamma(1 + x) - log x, and on
 * (9/4, 3) lgamma(x - 1) + log(x - 1), both relative where the terms are small (near 1 and 2 they cancel by at most
 * 1/gamma and 1/(1 - gamma)); from 3 the shift to 12 and Stirling, which no longer cancel much (lgamma(3) = log 2)
 */
static struct __libc_dd lgamma_positive(struct __libc_dd x)
{
	struct __libc_dd product;
	struct __libc_dd shifted;
	int n;

	if (x.hi >= 12.0)
		return stirling(x);
	if (x.hi < 3.0) {
		/* below 3/4 lgamma(1 + x) - log x, on (9/4, 3) lgamma(x - 1) + log(x - 1), one call of each kernel */
		struct __libc_dd value;
		bool below = x.hi < 0.75;
		bool above = x.hi > 2.25;

		shifted = below ? __libc_dd_add((struct __libc_dd){ .hi = 1.0, .lo = 0.0 }, x) : (above ? __libc_dd_add(x, (struct __libc_dd){ .hi = -1.0, .lo = 0.0 }) : x);
		value = lgamma_unit(shifted);
		if (below || above) {
			struct __libc_dd log = log_dd(below ? x : shifted);

			value = __libc_dd_add(value, below ? __libc_dd_negate(log) : log);
		}

		return value;
	}

	/* gamma(x) = gamma(x + n) / (x (x + 1) ... (x + n - 1)) with x + n >= 12 */
	n = (int)ceil(12.0 - x.hi);
	product = x;
	for (int k = 1; k < n; ++k)
		product = __libc_dd_multiply(product, __libc_dd_add(x, (struct __libc_dd){ .hi = k, .lo = 0.0 }));
	shifted = __libc_dd_add(x, (struct __libc_dd){ .hi = n, .lo = 0.0 });

	return __libc_dd_add(stirling(shifted), __libc_dd_negate(log_dd(product)));
}

/*
 * Positive x directly; negative x by the reflection
 * gamma(x) gamma(1 - x) = pi / sin(pi x): log pi - log|sin(pi x)| -
 * lgamma(1 - x), with 1 - x exact as a double-double and sin(pi x) from the
 * exact reduction x = n + f, |f| <= 1/2.
 */
struct __libc_dd __libc_lgamma_core(double x, int *sign)
{
	const struct __libc_dd log_pi = { .hi = 0x1.250d048e7a1bdp+0, .lo = 0x1.7abf2ad8d5088p-57 };
	double n;
	double f;
	double a;
	struct __libc_dd angle;
	struct __libc_dd sine;
	struct __libc_dd value;

	*sign = 1;
	if (x > 0)
		return lgamma_positive((struct __libc_dd){ .hi = x, .lo = 0.0 });

	n = roundeven(x);
	f = x - n;
	a = fabs(f);
	*sign = (f < 0) != (fmod(n, 2.0) != 0) ? -1 : 1;
	if (a <= 0.25) {
		angle = __libc_dd_product(a, __LIBC_PI.hi);
		angle = __libc_dd_fast_sum(angle.hi, angle.lo + a * __LIBC_PI.lo);
		sine = __libc_sin_core(angle);
	} else {
		angle = __libc_dd_product(0.5 - a, __LIBC_PI.hi);
		angle = __libc_dd_fast_sum(angle.hi, angle.lo + (0.5 - a) * __LIBC_PI.lo);
		sine = __libc_cos_core(angle);
	}
	value = __libc_dd_add(log_pi, __libc_dd_negate(log_dd(sine)));

	return __libc_dd_add(value, __libc_dd_negate(lgamma_positive(__libc_dd_sum(1.0, -x))));
}
