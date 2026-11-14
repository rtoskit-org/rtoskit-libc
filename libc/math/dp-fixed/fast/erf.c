/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * erf.c
 *
 * Created on: Nov 14, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include <math.h>
#include <stdint.h>

#include "dp-fixed-fast.h"
#include "internal/math-fast.h"

/*
 * erf(a + u/4) for a = (j + 2)/4, j = 0..21, and u in [0, 1): b0 = erf(a) in Q64, then b1..b12 signed in Q63, the
 * coefficients of u^k, fitted here by least squares on Chebyshev nodes (within 2^-61.94 of erf, their rounding included)
 */
static const uint64_t pieces[22][13] = {
	{ 0x853f7ae0c76e915eull, 0x1c1efca49a5011dcull, 0xfc7c206b6cb5dfd8ull, 0xffb502b39e6713c0ull, 0x00176f27de6320a6ull, 0x00003bfdd77ce2b6ull, 0xffff9983adcbd052ull, 0x000002c482bf0577ull, 0x0000014929523decull, 0xffffffee59f8e33cull, 0xfffffffcafdb2110ull, 0x00000000502d696dull, 0x000000000156aac5ull },
	{ 0xb60e4bace872fb62ull, 0x1492e42d78d2c4c8ull, 0xfc2475377958719dull, 0x000db742c8fc0b50ull, 0x001349b5ea97a912ull, 0xfffe4bd5e7b1b935ull, 0xffffc8f70faa799dull, 0x000009705b5384faull, 0x0000004b6596e496ull, 0xffffffdf8853647eull, 0x0000000057460673ull, 0x00000000588f2b22ull, 0xfffffffff9ba902eull },
	{ 0xd7bb3d3a08445637ull, 0x0d488f84b7de129cull, 0xfcaddc1ed20889d6ull, 0x0046d852c3d3615aull, 0x0008db0a5888c2e2ull, 0xfffdc93d69816b66ull, 0x000009722ecaf943ull, 0x000007c257414718ull, 0xffffff6380a78122ull, 0xfffffff081f42d3full, 0x000000028e2eb0faull, 0x0000000004d99b4bull, 0xfffffffffb616fd3ull },
	{ 0xec432ecc55f00407ull, 0x079194beb72dac43ull, 0xfda2818466c1c6fcull, 0x0055c74071703e63ull, 0xffff362b2c2f0311ull, 0xfffe7d7e319d92a4ull, 0x00002ba001f4f995ull, 0x000001db3f60b772ull, 0xffffff455552af0cull, 0x0000000724220fb1ull, 0x00000001abbfcd20ull, 0xffffffffce51d6ddull, 0x0000000000eb21adull },
	{ 0xf752aab89bd6fd8full, 0x03ce46e4d7049520ull, 0xfe92a56a2f5e471eull, 0x00470a80afadbf2bull, 0xfffa4a95a8bc9f26ull, 0xffff8637236a74d0ull, 0x000027947ad0c5a2ull, 0xfffffd9251be6627ull, 0xffffffb2941fd4edull, 0x0000000e01f763dcull, 0xffffffffcf5d48e8ull, 0xffffffffde51c1fcull, 0x0000000002dd172aull },
	{ 0xfc9683bfc6ab698bull, 0x01b0553032210174ull, 0xff42dabaea118924ull, 0x002e29187ab0102full, 0xfff9d7c96aedbdb3ull, 0x000036444b258c90ull, 0x0000125b36d59b6dull, 0xfffffce5e01ffc74ull, 0x00000017e7efc8d4ull, 0x0000000758f66f72ull, 0xffffffff12945f4aull, 0x00000000026cc9a0ull, 0x00000000012ef9ecull },
	{ 0xfecd70a13caf1997ull, 0x00a94dcf467cd0feull, 0xffab59185cc194d7ull, 0x0018b0ae3a47c4a9ull, 0xfffb974e9a276742ull, 0x00006b39d02572feull, 0x000000f0c95dfcedull, 0xfffffe452045f492ull, 0x0000003420993663ull, 0xffffffff999d9163ull, 0xffffffff7396d429ull, 0x000000000f46609bull, 0xffffffffff845167ull },
	{ 0xffa023b018cda1f1ull, 0x003a828753bfd2c6ull, 0xffdf1693e0e41a53ull, 0x000b1f7b0f410ea5ull, 0xfffd8eacf9b1c98eull, 0x0000574ec5e93648ull, 0xfffffa0d47e39b33ull, 0xffffffa81ef48864ull, 0x00000020c0e4f87full, 0xfffffffcf8a7e935ull, 0xfffffffffa955566ull, 0x00000000083c3a2eull, 0xffffffffff524845ull },
	{ 0xffe5547a64df5362ull, 0x0011d83170fbf6f6ull, 0xfff4d8e1196286e8ull, 0x000446768311972bull, 0xfffee5764684560bull, 0x0000321d0247f386ull, 0xfffffa44c927ccf9ull, 0x0000004717d5159aull, 0x000000088b85e3c5ull, 0xfffffffdf1eb8c22ull, 0x000000002a6e6c3aull, 0xffffffffffff073eull, 0xffffffffffc7975eull },
	{ 0xfff967d8229ecb35ull, 0x0004cd833348f6cdull, 0xfffcb2b5ccbdd6b4ull, 0x000169cf466cc57full, 0xffff953cf631b122ull, 0x00001693595dc586ull, 0xfffffc9b1553a9e0ull, 0x00000054ad66dc61ull, 0xfffffffd15984c1aull, 0xffffffff6a3d319cull, 0x000000001d3831d2ull, 0xfffffffffdac7fc5ull, 0x000000000010c11full },
	{ 0xfffe8d6209afcbddull, 0x00012408e9ba3328ull, 0xffff24f950b45984ull, 0x0000676dd2c74a0aull, 0xffffddc6f49c10b1ull, 0x00000853daaafad9ull, 0xfffffe7d0dc6ff3eull, 0x0000003331727a66ull, 0xfffffffb9555f42aull, 0x000000001d429afbull, 0x00000000081ad8daull, 0xfffffffffea0dd5bull, 0x000000000016ceb5ull },
	{ 0xffffb7cfb3f2abc4ull, 0x00003d36bd1a019eull, 0xffffce43865ade8cull, 0x000019aa489e3fcbull, 0xfffff69c150859aeull, 0x00000292149b39a6ull, 0xffffff75d5c95fffull, 0x000000164803601aull, 0xfffffffd52fe66f7ull, 0x00000000367a83f6ull, 0xfffffffffea76be0ull, 0xffffffffffb35f47ull, 0x000000000008fa74ull },
	{ 0xfffff3886ab2f394ull, 0x00000b52cb90cd4aull, 0xfffff6178de14c53ull, 0x0000058b33a99044ull, 0xfffffdc7f11bbdd5ull, 0x000000ac35d83b5cull, 0xffffffd73d568b1dull, 0x00000007a0a457c1ull, 0xfffffffee093f80bull, 0x00000000203357d3ull, 0xfffffffffd84dd00ull, 0x000000000016b3c2ull, 0x000000000000ad2cull },
	{ 0xfffffe178b8f1b0cull, 0x000001d9371e2ff8ull, 0xfffffe445c53b307ull, 0x0000010b6a7b0f26ull, 0xffffff8be429e755ull, 0x0000002686d797c3ull, 0xfffffff5e5424384ull, 0x00000002221c025bull, 0xffffffffa2a5d060ull, 0x000000000cd078beull, 0xfffffffffea1b41dull, 0x00000000001ab521ull, 0xfffffffffffee186ull },
	{ 0xffffffbdc88bb10bull, 0x000000442c5222bbull, 0xffffffbbd3addd47ull, 0x0000002c074a60f5ull, 0xffffffeb67f1dc6full, 0x00000007697c389cull, 0xfffffffddf5f8858ull, 0x000000007f5e8f2cull, 0xffffffffe7749954ull, 0x0000000003e6cdf3ull, 0xffffffffff7f2345ull, 0x00000000000ca32eull, 0xffffffffffff4443ull },
	{ 0xfffffff80d45b012ull, 0x00000008aad04833ull, 0xfffffff6ca82b34bull, 0x0000000657a9bf6cull, 0xfffffffcd28b1a1dull, 0x000000013b4679d8ull, 0xffffffff9de5e307ull, 0x0000000019169825ull, 0xfffffffffaa6e7b7ull, 0x0000000000f47650ull, 0xffffffffffdbecfaull, 0x0000000000041995ull, 0xffffffffffffb9cbull },
	{ 0xffffffff27d180e9ull, 0x00000000f8f1a91eull, 0xfffffffee7f021bfull, 0x00000000ccdc3324ull, 0xffffffff9299cd7eull, 0x000000002d6373c8ull, 0xfffffffff0cd8247ull, 0x0000000004358280ull, 0xffffffffff053ed3ull, 0x00000000003156acull, 0xfffffffffff7e9f6ull, 0x0000000000010519ull, 0xffffffffffffece9ull },
	{ 0xffffffffebacebefull, 0x0000000018a61746ull, 0xffffffffe2bac45dull, 0x0000000016a8aeb9ull, 0xfffffffff327f43eull, 0x0000000005ad0e9eull, 0xfffffffffdf7a402ull, 0x00000000009ae752ull, 0xffffffffffd90789ull, 0x0000000000085672ull, 0xfffffffffffe829bull, 0x0000000000003425ull, 0xfffffffffffffbefull },
	{ 0xfffffffffe4f3e59ull, 0x0000000002275e9eull, 0xfffffffffd4ec9baull, 0x000000000232db41ull, 0xfffffffffeae92c7ull, 0x00000000009e28ceull, 0xffffffffffc3b9c4ull, 0x0000000000132b17ull, 0xfffffffffffad38dull, 0x0000000000013115ull, 0xffffffffffffc552ull, 0x0000000000000891ull, 0xffffffffffffff4cull },
	{ 0xffffffffffe02a0full, 0x00000000002a845eull, 0xffffffffffc83245ull, 0x00000000002ff142ull, 0xffffffffffe1b34cull, 0x00000000000f0221ull, 0xfffffffffff9f069ull, 0x0000000000020c7bull, 0xffffffffffff690full, 0x0000000000002526ull, 0xfffffffffffff862ull, 0x000000000000012dull, 0xffffffffffffffe6ull },
	{ 0xfffffffffffdedd0ull, 0x000000000002e4b1ull, 0xfffffffffffc058dull, 0x0000000000039626ull, 0xfffffffffffd9dfeull, 0x0000000000013e4aull, 0xffffffffffff784bull, 0x000000000000308dull, 0xfffffffffffff12dull, 0x00000000000003e0ull, 0xffffffffffffff29ull, 0x0000000000000023ull, 0xfffffffffffffffdull },
	{ 0xffffffffffffe181ull, 0x0000000000002c7bull, 0xffffffffffffc00full, 0x0000000000003c5aull, 0xffffffffffffd5f4ull, 0x000000000000170bull, 0xfffffffffffff5a9ull, 0x00000000000003e7ull, 0xfffffffffffffebeull, 0x0000000000000059ull, 0xffffffffffffffecull, 0x0000000000000003ull, 0x0000000000000000ull }
};

/* G(w) = (2/sqrt(pi) - erf(x)/x)/w for w = 4 x^2 in [0, 1), nine signed Q64 coefficients fitted here (w G within 2^-63 of the rest, relatively) */
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

/*
 * The ends (C23 7.12.8.1, F.10.5.1): +-0 and NaN return themselves, +-inf +-1; below 2^-60, x 2/sqrt(pi) by the strict
 * file's scaled product (underflow for a subnormal result); from 6, +-(1 - tiny) (erfc 6 is below 2^-55)
 */
[[gnu::noinline]] static double erf_ends(double x)
{
	const struct __libc_dd two_sqrtpi = { .hi = 0x1.20dd750429b6dp+0, .lo = 0x1.1ae3a914fed80p-56 };
	volatile double tiny = 0x1p-80;

	if (isnan(x) || x == 0)
		return x + x;
	if (isinf(x))
		return __builtin_copysign(1.0, x);
	if (__builtin_fabs(x) < 0x1p-60)
		return __libc_dd_scale_product(x, two_sqrtpi);

	/* At run time, so that it is inexact */
	return __builtin_copysign(1.0 - tiny, x);
}

/*
 * The fast erf of the soft-double cores (PLAN D162, D205), in 64-bit fixed point with one rounding and no soft-double
 * operation on the common path, 2^-60 <= |x| < 6:
 * - below 1/2, x K(w) with w = 4 x^2 in Q64 and K = 2/sqrt(pi) - w G(w) in Q62, relative however small x is;
 * - from 1/2, 22 pieces of width 1/4, erf(a) + u Q(u) with u = 4 |x| - floor(4 |x|) exact in Q64 from x's bits and Q's
 *   twelve coefficients signed in Q63 (signed Horner's rule, the products truncated toward zero): from 1/2 erf is at
 *   least 0.52, so the few units of 2^-63 stay below 2^-58 of the result.
 */
double erf(double x)
{
	uint64_t bits = __libc_double_bits(x);
	uint64_t sign = bits & 0x8000000000000000ull;
	uint64_t magnitude = bits ^ sign;
	int32_t e = (int32_t)(magnitude >> 52) - 1023;
	uint64_t significand = (magnitude & 0x000fffffffffffffull) | 0x0010000000000000ull;
	const uint64_t *c;
	uint64_t product;
	uint64_t u;
	uint64_t w;
	int64_t q;

	/* 2^-60 <= |x| < 6: the bits of a positive double grow with it */
	if (magnitude - 0x3c30000000000000ull >= 0x4018000000000000ull - 0x3c30000000000000ull)
		return erf_ends(x);

	if (e < -1) {
		/* M = m 2^-63, M^2 in Q62 = high(m, m), w = 4 x^2 = M^2 2^(2e + 2) in Q64 */
		uint64_t m = significand << 11;

		w = -2 * e - 4 < 64 ? __libc_trig_high64(m, m) >> (-2 * e - 4) : 0;
		q = step(series[7], w, series[8]);
		q = step(series[6], w, q);
		q = step(series[5], w, q);
		q = step(series[4], w, q);
		q = step(series[3], w, q);
		q = step(series[2], w, q);
		q = step(series[1], w, q);
		q = step(series[0], w, q);
		/* x K = (m K 2^-64) 2^(e - 61), K in Q62 (G is positive) */
		return __libc_dp_fixed_normal(sign, e + 2, __libc_trig_high64(m, 0x48375d410a6db447ull - (__libc_trig_high64(w, (uint64_t)q) >> 2)));
	}

	/* 4 |x| = significand 2^(e - 50): its integer part picks the piece, its fraction is u */
	c = pieces[(significand >> (50 - e)) - 2];
	u = significand << (14 + e);
	q = step((int64_t)c[11], u, (int64_t)c[12]);
	q = step((int64_t)c[10], u, q);
	q = step((int64_t)c[9], u, q);
	q = step((int64_t)c[8], u, q);
	q = step((int64_t)c[7], u, q);
	q = step((int64_t)c[6], u, q);
	q = step((int64_t)c[5], u, q);
	q = step((int64_t)c[4], u, q);
	q = step((int64_t)c[3], u, q);
	q = step((int64_t)c[2], u, q);
	q = step((int64_t)c[1], u, q);
	/* erf = b0 + u q, in Q64 (q in Q63): at least 0.52, so bit 63 is set */
	product = q < 0 ? -(__libc_trig_high64(u, -(uint64_t)q) << 1) : __libc_trig_high64(u, (uint64_t)q) << 1;

	return __libc_dp_fixed_double(sign, -1, c[0] + product);
}
