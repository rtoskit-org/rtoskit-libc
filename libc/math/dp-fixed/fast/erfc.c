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

#include "dp-fixed-fast.h"
#include "internal/math-fast.h"

/*
 * R(a) = erfc(a) e^(a^2) at a = (j + 2 + u)/4, j = 0..13, u in [0, 1): b0 in Q64, then b1..b11 signed in Q63, the
 * coefficients of u^k, fitted here by least squares on Chebyshev nodes (within 2^-60.42 of R with the fixed-point
 * evaluation's truncations, relatively 2^-57.6 where R is least, 0.137 at 4); then G(w) = R(1/sqrt(w))/sqrt(w) at
 * w = (j + u)/64, j = 0..3, b1..b9 (within 2^-61.10; G is about 1/sqrt(pi))
 */
static const uint64_t r_pieces[14][12] = {
	{ 0x9d9de1e4c587964cull, 0xef980d9c137a1e39ull, 0x02dff0c2a89a7514ull, 0xff8e54a1644598f0ull, 0x000fe4d02aca3417ull, 0xfffdf3f4dc224a74ull, 0x00003eee7a66f526ull, 0xfffff8e3e4f5a5cdull, 0x000000c29bd802ceull, 0xffffffec62904a1bull, 0x00000001c2fa75cfull, 0xffffffffe496fa76ull },
	{ 0x81c6aa751ec19c24ull, 0xf4399155708d7507ull, 0x01d90093ae103824ull, 0xffbd8620afce95ddull, 0x00088c97adab0674ull, 0xfffefab29946fcb9ull, 0x00001d43a289c9d3ull, 0xfffffce6dce23973ull, 0x0000004fcc06e2feull, 0xfffffff865b64388ull, 0x00000000a68b4221ull, 0xfffffffff639e3faull },
	{ 0x6d761e03e55ac85full, 0xf741d8e0741fd8a1ull, 0x013c27283c32ac11ull, 0xffd76f900ee17c1aull, 0x0004cf2b43a699d9ull, 0xffff778187a873e5ull, 0x00000e465aceef6bull, 0xfffffe951216cf6dull, 0x000000226310faffull, 0xfffffffce6c1e683ull, 0x0000000040adf77bull, 0xfffffffffc55e973ull },
	{ 0x5e29a484e1d9e802ull, 0xf95154c9015d3e9eull, 0x00dab7a2f77be6b5ull, 0xffe649ca539e96caull, 0x0002d144b4c3f7dbull, 0xffffb59a727b4770ull, 0x00000746d9a46f1eull, 0xffffff523bc7ace2ull, 0x0000000f854167cfull, 0xfffffffead245957ull, 0x000000001a55b6e9ull, 0xfffffffffe8f4d24ull },
	{ 0x52536bfe8ab5a13aull, 0xfac399deeecd426bull, 0x009bf5138de2a134ull, 0xffef23ae2d6c39c8ull, 0x0001b65944eff849ull, 0xffffd5d8252720c0ull, 0x000003dce0774ac9ull, 0xffffffa93df735e1ull, 0x000000075035516full, 0xffffffff68c01bdaull, 0x000000000b354568ull, 0xffffffffff6885e8ull },
	{ 0x48f3f0bc771f809dull, 0xfbcf0ab1eee6ee2cull, 0x00723433bc3e0248ull, 0xfff49a5680dbd9cbull, 0x000113648a10a327ull, 0xffffe7408e9d3e4aull, 0x00000220d78f22acull, 0xffffffd2f9330e6cull, 0x00000003962b48ddull, 0xffffffffb9ab0942ull, 0x0000000004f8dcb7ull, 0xffffffffffbf1fecull },
	{ 0x41619c6d4b1dd7b1ull, 0xfc951f96205811baull, 0x00559cae7a84f6c8ull, 0xfff8158b14dab7e0ull, 0x0000b248390a3de0ull, 0xfffff0feb8be23e0ull, 0x000001369fedffdfull, 0xffffffe7c7fac6f5ull, 0x00000001d34d3f6eull, 0xffffffffde06032full, 0x00000000024afe15ull, 0xffffffffffe31c4cull },
	{ 0x3b2888dac4f26396ull, 0xfd2b1e5a89917de3ull, 0x00418559c389698full, 0xfffa5df019b99e5dull, 0x0000769655583cbdull, 0xfffff6a16d739bc3ull, 0x000000b6bb9c9d9bull, 0xfffffff2894d5616ull, 0x00000000f6273606ull, 0xffffffffeefe28e8ull, 0x000000000118afe9ull, 0xfffffffffff2a82bull },
	{ 0x35f767e76e687598ull, 0xfd9ef2501fca6f5dull, 0x003312b14f51c91cull, 0xfffbe737e25fdb94ull, 0x000050d70138642eull, 0xfffff9fd8c5dfd01ull, 0x0000006ea297894bull, 0xfffffff848b53c81ull, 0x0000000085dc8511ull, 0xfffffffff7336530ull, 0x00000000008ad705ull, 0xfffffffffff99f78ull },
	{ 0x3193d6d1fe4fad22ull, 0xfdf9f50fd99fece5ull, 0x0028773175907032ull, 0xfffcf62ca99e341dull, 0x00003858e5fad6e2ull, 0xfffffc0cc37c76f1ull, 0x00000044c6eb349aull, 0xfffffffb73b7f17aull, 0x000000004afa7586ull, 0xfffffffffb4e1f68ull, 0x000000000046e1d2ull, 0xfffffffffffcda62ull },
	{ 0x2dd304fa453a599cull, 0xfe42951b2eb4e912ull, 0x002087fc35318190ull, 0xfffdb4df3c8af5e2ull, 0x00002813985da129ull, 0xfffffd58426c8601ull, 0x0000002bce71d2b4ull, 0xfffffffd3fb7334dull, 0x000000002b29c29dull, 0xfffffffffd6c1be7ull, 0x000000000025459eull, 0xfffffffffffe665bull },
	{ 0x2a94f411a97689f5ull, 0xfe7d57add47975f4ull, 0x001a7eddc9ee6421ull, 0xfffe3db4b63b862eull, 0x00001d0858579e13ull, 0xfffffe2da14471ceull, 0x0000001c882ebb2full, 0xfffffffe4ba3a7b3ull, 0x00000000197cc6fdull, 0xfffffffffe8be59dull, 0x0000000000142594ull, 0xffffffffffff29c6ull },
	{ 0x27c153341dd227e7ull, 0xfead7a2d14e108c7ull, 0x0015d581133378ebull, 0xfffea1a281676717ull, 0x000015632136d70aull, 0xfffffeb9f5429eeeull, 0x00000012f839c0b5ull, 0xfffffffeeb949378ull, 0x000000000f68c88cull, 0xffffffffff28e675ull, 0x00000000000b2ca4ull, 0xffffffffffff8cfcull },
	{ 0x25455e3382925f03ull, 0xfed559afc5325eefull, 0x00122f0664f3cbf7ull, 0xfffeebcb51fa78deull, 0x00000fff8195060eull, 0xffffff18190af7c3ull, 0x0000000cda867345ull, 0xffffffff4d4468d9ull, 0x000000000985cde8ull, 0xffffffffff80bf18ull, 0x0000000000065958ull, 0xffffffffffffc0b6ull },
};

static const uint64_t g_pieces[4][10] = {
	{ 0x906eba8214db6888ull, 0xff6f91457deb2694ull, 0x000362985f0c3b06ull, 0xffffde260c4cdba6ull, 0x000001d9eb3de408ull, 0xffffffdeadc84f62ull, 0x00000002dc52d6ffull, 0xffffffffb6adc754ull, 0x0000000007af5848ull, 0xffffffffff694967ull },
	{ 0x8f5462004da2c9e4ull, 0xff75f7b8d3ab2315ull, 0x000306fe78515847ull, 0xffffe471303dc059ull, 0x000001560295f977ull, 0xffffffeb3882520bull, 0x00000001813642e2ull, 0xffffffffe00dd458ull, 0x0000000002c9f82aull, 0xffffffffffd068d0ull },
	{ 0x8e462ad68404c8b9ull, 0xff7bb801b63465b1ull, 0x0002bb9a885be8adull, 0xffffe913b65fd075ull, 0x00000100fd040e3aull, 0xfffffff225c4c83eull, 0x00000000e028dee5ull, 0xffffffffeff93dc8ull, 0x0000000001359071ull, 0xffffffffffed96ecull },
	{ 0x8d42e61e56013b61ull, 0xff80ee3574c08664ull, 0x00027c5d045fab17ull, 0xffffec9cb412304eull, 0x000000c6f6eb2c59ull, 0xfffffff650dc6e5cull, 0x000000008bd227cfull, 0xfffffffff729a52cull, 0x0000000000970fd6ull, 0xfffffffffff7da0aull },
};

/* G(w) = (2/sqrt(pi) - erf(x)/x)/w for w = 4 x^2 in [0, 1), nine signed Q64 coefficients: the fast erf's series (dp-fixed/fast/erf.c) */
static const int64_t series[9] = {
	0x1812746b0379e6c0ll,
	-130093260709477623ll,
	0x001b82ce31286cd5ll,
	-376427258917055ll,
	0x00000e016d99b115ll,
	-542922731028ll,
	0x00000003e985787ell,
	-461323120ll,
	0x00000000009ec872ll,
};

/* c + u p in c's units for u in Q64 (unsigned) and a signed p, the product truncated toward zero */
static inline int64_t step(int64_t c, uint64_t u, int64_t p)
{
	return c + (p < 0 ? -(int64_t)__libc_trig_high64(u, -(uint64_t)p) : (int64_t)__libc_trig_high64(u, (uint64_t)p));
}

/* b0 + u Q(u) in Q64 for a row of n + 1 coefficients (signed Horner's rule from b_n) */
static inline uint64_t piece(const uint64_t *c, int n, uint64_t u)
{
	int64_t q = step((int64_t)c[n - 1], u, (int64_t)c[n]);

	for (int k = n - 2; k > 0; --k)
		q = step((int64_t)c[k], u, q);

	return c[0] + (q < 0 ? -(__libc_trig_high64(u, -(uint64_t)q) << 1) : __libc_trig_high64(u, (uint64_t)q) << 1);
}

/*
 * The ends (C23 7.12.8.2, F.10.5.2): NaN returns itself, +inf +0 and -inf 2; below 2^-56, 1 - x (2/sqrt(pi) x is below a
 * quarter of an ulp of 1); below -6, 2 - tiny (erfc 6 is below 2^-55); above 27.3, +0 (erfc is below 2^-1075 from 27.23)
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
 * e^(-a^2) as Y 2^(*n - 63), Y in Q63 (exp's kernel), for a = m 2^(e - 52), m the 53-bit significand, -1 <= e <= 4: a^2
 * log2(e) = t = n + f 2^-64 from m^2 exactly (106 bits, H 2^64 + L) times log2(e) in Q127 (c_hi 2^64 + c_lo), the
 * product's top bits T = H c_hi + high(L c_hi) + high(H c_lo) in units of 2^(2 e - 103) (the dropped L c_lo below one
 * unit), so that t's fraction keeps 31 bits more than f; then -t = n' + g, 2^(g 2^-64) by exp's kernel
 */
static inline uint64_t erfc_exp(uint64_t m, int32_t e, int32_t *n)
{
	constexpr uint64_t c_hi = 0xb8aa3b295c17f0bbull;
	constexpr uint64_t c_lo = 0xbe87fed0691d3e88ull;
	uint32_t m0 = (uint32_t)m;
	uint32_t m1 = (uint32_t)(m >> 32);
	uint64_t p00 = __libc_trig_wide(m0, m0);
	uint64_t p01 = __libc_trig_wide(m0, m1);
	uint64_t lo = p00 + (p01 << 33);
	uint64_t h = __libc_trig_wide(m1, m1) + (p01 >> 31) + (lo < p00);
	uint64_t t_hi;
	uint64_t t_lo;
	uint64_t a_hi;
	uint64_t a_lo;
	uint64_t add;
	uint64_t f;
	int s = 39 - 2 * e;

	/* H c_hi with H below 2^42, as (H mod 2^32) c_hi + (H >> 32) c_hi 2^32 */
	__libc_dp_fixed_k_product((uint32_t)h, c_hi, &t_hi, &t_lo);
	__libc_dp_fixed_k_product((uint32_t)(h >> 32), c_hi, &a_hi, &a_lo);
	t_lo += a_lo << 32;
	t_hi += (a_hi << 32 | a_lo >> 32) + (t_lo < (a_lo << 32));
	add = __libc_trig_high64(lo, c_hi);
	t_lo += add;
	t_hi += t_lo < add;
	add = __libc_trig_high64(h, c_lo);
	t_lo += add;
	t_hi += t_lo < add;

	*n = (int32_t)(t_hi >> s);
	f = (t_hi << (64 - s)) | (t_lo >> s);
	f = __libc_dp_fixed_negate(n, f);

	return __libc_dp_fixed_exp2_fraction(f);
}

/* The double of v 2^(e - 63), v nonzero, rounded once to nearest, onto the subnormal grid below 2^-1022 */
static inline double erfc_result(uint64_t v, int32_t e)
{
	int k = __builtin_clzll(v);
	int32_t shift;

	v <<= k;
	e -= k;
	if (e >= -1022)
		return __libc_dp_fixed_double(0, e, v);
	shift = -1022 - e;
	/* Below 2^-1074: from 2^-1075 (no tie there) the smallest subnormal, below it +0 */
	if (shift >= 53)
		return __libc_double_from(shift == 53 ? 1 : 0);

	return __libc_double_from((v >> (11 + shift)) + ((v >> (10 + shift)) & 1));
}

/*
 * The fast erfc of the soft-double cores (PLAN D162, D219, D224), in 64-bit fixed point with no soft-double operation on
 * the common path, within 1 ulp:
 * - below 1/2 in magnitude, 1 -+ x K(w) in Q62, x K(w) as the fast erf's (w = 4 x^2 in Q64, K = 2/sqrt(pi) - w G(w) in
 *   Q62);
 * - from 1/2 to 27.3, erfc(a) = e^(-a^2) R(a), a = |x|: e^(-a^2) = Y 2^(n - 63) by erfc_exp; below 4, R by fourteen
 *   pieces of width 1/4 in u = 4 a - floor(4 a), exact from a's bits; from 4, R = v G(v^2), v = 1/a = y 2^(-64 - e) by
 *   __libc_dp_fixed_reciprocal (2^-63 relatively), w = v^2 in Q64 and four pieces of width 1/64 in it; the products'
 *   high halves, rounded once (onto the subnormal grid from 26.55);
 * - for negative x, 2 - erfc(a) in Q62 (the result is at least 1.5); below -6, 2 - tiny.
 * These cores round to nearest and raise no flags (D44). C23 7.12.8.2.
 */
double erfc(double x)
{
	uint64_t bits = __libc_double_bits(x);
	uint64_t sign = bits & 0x8000000000000000ull;
	uint64_t magnitude = bits ^ sign;
	int32_t e = (int32_t)(magnitude >> 52) - 1023;
	uint64_t m = (magnitude & 0x000fffffffffffffull) | 0x0010000000000000ull;
	uint64_t y;
	uint64_t r;
	uint64_t f;
	int32_t n;

	/* 2^-56 <= |x| < 27.3, and x above -6: the bits of a positive double grow with it */
	if (magnitude - 0x3c70000000000000ull >= 0x403b4ccccccccccdull - 0x3c70000000000000ull || (sign != 0 && magnitude >= 0x4018000000000000ull))
		return erfc_ends(x);

	if (e < -1) {
		/* M = m 2^-63 with m now in [2^63, 2^64), x K = high(m, K) 2^(e - 61), in Q62 shifted by -e - 1 */
		uint64_t w;
		int64_t q;

		m <<= 11;
		w = -2 * e - 4 < 64 ? __libc_trig_high64(m, m) >> (-2 * e - 4) : 0;
		q = step(series[7], w, series[8]);
		for (int k = 6; k >= 0; --k)
			q = step(series[k], w, q);
		r = __libc_trig_high64(m, 0x48375d410a6db447ull - (__libc_trig_high64(w, (uint64_t)q) >> 2)) >> (-e - 1);

		return __libc_dp_fixed_normal(0, 1, sign != 0 ? 0x4000000000000000ull + r : 0x4000000000000000ull - r);
	}

	y = erfc_exp(m, e, &n);
	if (e < 2) {
		/* 4 a = m 2^(e - 50): its integer part picks the piece, its fraction is u */
		r = piece(r_pieces[(m >> (50 - e)) - 2], 11, m << (14 + e));
	} else {
		uint64_t v = __libc_dp_fixed_reciprocal(m << 11);
		uint64_t w = __libc_trig_high64(v, v) >> (2 * e);

		r = __libc_trig_high64(v, piece(g_pieces[w >> 58], 9, w << 6));
		n -= e;
	}
	f = __libc_trig_high64(y, r);
	if (sign == 0)
		return erfc_result(f, n);

	/* 2 - f 2^(n - 63) in Q62 (n <= 0: erfc(a) is below 1/2) */
	return __libc_dp_fixed_normal(0, 1, 0x8000000000000000ull - (1 - n < 64 ? f >> (1 - n) : 0));
}
