/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * lgamma.c
 *
 * Created on: Nov 17, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include <math.h>
#include <stdint.h>

#include "dp-fixed-fast.h"
#include "internal/math-fast.h"

/*
 * h(x) = lgamma(x)/((x - 1)(x - 2)) on [1/2, 8), smooth and positive, in 32 pieces of eight a binade (widths 1/16 to
 * 1/2) at x = (start + u width), u in [0, 1) in Q64 from x's fraction bits below the top three: b0 in Q64, then b1..b10
 * signed in Q63, the coefficients of u^k, fitted here by least squares on Chebyshev nodes (within 2^-59.39 absolutely
 * with the evaluation's truncations; h is at least 0.2, so 2^-57 relatively)
 */
static const uint64_t pieces[32][11] = {
	{ 0xc35e0309a6bd35bdull, 0xfb56249801d727a5ull, 0x005be1d2c9b8caf5ull, 0xfff79d24167b3a7full, 0x0000d45e15d7816bull, 0xffffea002f22262bull, 0x000002593c9b8b78ull, 0xffffffbe7f0351cbull, 0x00000007295847eeull, 0xffffffff457ad394ull, 0x000000000cfef93full },
	{ 0xbab2cb2063a8c305ull, 0xfbf7afa33e8917eaull, 0x0046f600f3ad390eull, 0xfffa39fc4ddaf7e4ull, 0x0000824914d0c2b0ull, 0xfffff3fa0c72597eull, 0x00000124a3426b9aull, 0xffffffe39aab6cffull, 0x00000002c5d61789ull, 0xffffffffbead1f7aull, 0x00000000043ab689ull },
	{ 0xb32578fd33a9195bull, 0xfc761cc9bc43b374ull, 0x0038488749a2e386ull, 0xfffbde67d75a33ddull, 0x0000541472fc4151ull, 0xfffff900d94038f3ull, 0x00000099863c4551ull, 0xfffffff291efc68eull, 0x000000012f3e7ba0ull, 0xffffffffe68a4573ull, 0x00000000018a2181ull },
	{ 0xac7a9bb3e5a18075ull, 0xfcdb79a9a6f7e264ull, 0x002d9e44afdf90afull, 0xfffcf32544131782ull, 0x0000388928e685b3ull, 0xfffffbb748d31a24ull, 0x0000005595e9c35eull, 0xfffffff92f0d3c99ull, 0x000000008c50953dull, 0xfffffffff52c70f7ull, 0x00000000009d3b7dull },
	{ 0xa6871afa8d0c589aull, 0xfd2e5e31b6ba156bull, 0x0025a496dd3affb9ull, 0xfffdb04f52e7d7acull, 0x0000275457610be1ull, 0xfffffd43a0985800ull, 0x000000322b20f5faull, 0xfffffffc552a214bull, 0x00000000455cb25bull, 0xfffffffffb0cfd40ull, 0x000000000043b24eull },
	{ 0xa12acab7cca9ec57ull, 0xfd7349064251a161ull, 0x001f88cd9b6cd142ull, 0xfffe35bda5fabce5ull, 0x00001c27654d6317ull, 0xfffffe309f1b2e89ull, 0x0000001ead007a30ull, 0xfffffffded7c4f6cull, 0x00000000243fca30ull, 0xfffffffffd985291ull, 0x00000000001f16c2ull },
	{ 0x9c4d0ec455d568e3ull, 0xfdad6415b5d214d3ull, 0x001ac27d68538e9cull, 0xfffe9665a0ca6b3bull, 0x000014a6efc42849ull, 0xfffffec405d50cf9ull, 0x0000001371c4ef9full, 0xfffffffec76adaeeull, 0x0000000013dd61d9ull, 0xfffffffffec4ca6bull, 0x00000000000f16b3ull },
	{ 0x97daafb056aa0bc0ull, 0xfddef91df808ff75ull, 0x0016f64c2c766a0eull, 0xfffede07b93ea2c7ull, 0x00000f78dad79485ull, 0xffffff22d858cce5ull, 0x0000000cb6cfcb33ull, 0xffffffff410ef11cull, 0x000000000b575a0cull, 0xffffffffff572b2eull, 0x000000000007aeb5ull },
	{ 0x93c467e37db0c78full, 0xfc13734d0e572d32ull, 0x004f9514811edd0eull, 0xfff8a1706575a514ull, 0x0000bce7cd58b602ull, 0xffffec34e42dfd26ull, 0x000002229f5947bcull, 0xffffffc3e5356ba7ull, 0x000000069f680fbaull, 0xffffffff52784904ull, 0x000000000c237972ull },
	{ 0x8c7d1199cfba9792ull, 0xfc9f1dc946aa0709ull, 0x003d3ca1b9835bf0ull, 0xfffaf2dcecb82ccaull, 0x00007344109687a5ull, 0xfffff53fd45209c9ull, 0x000001084b0db76dull, 0xffffffe61ed004edull, 0x000000028c1632dbull, 0xffffffffc39c2164ull, 0x0000000003ed30fbull },
	{ 0x862c7f12ee515d16ull, 0xfd0c0c81ea42af25ull, 0x00306aeba058e07full, 0xfffc6604390a5ddbull, 0x00004a0d9fa2d874ull, 0xfffff9c6e6484b99ull, 0x00000089d927225eull, 0xfffffff3d590e4a4ull, 0x0000000114e24c1eull, 0xffffffffe899fae2ull, 0x00000000016bd7f1ull },
	{ 0x809ec29cdcb3cc0cull, 0xfd63206ca49c15daull, 0x0027223e84a6232eull, 0xfffd59506ed97bcaull, 0x0000319b672f7060ull, 0xfffffc34f557e98bull, 0x0000004c74908522ull, 0xfffffff9dbe217c1ull, 0x000000007f687127ull, 0xfffffffff61abe08ull, 0x00000000009061c5ull },
	{ 0x7bae56c253064534ull, 0xfdaa25f4c853eee2ull, 0x002035ec54524e43ull, 0xfffdff0aaf867acdull, 0x0000226684679232ull, 0xfffffd964858ba2eull, 0x0000002c9de6dce5ull, 0xfffffffcb6415f22ull, 0x000000003eaa073eull, 0xfffffffffb7fc5aeull, 0x00000000003ddb54ull },
	{ 0x773f4ce6a2001664ull, 0xfde50d6d79516d09ull, 0x001aebb0c81e4e3cull, 0xfffe73992db2620dull, 0x0000188f37b5a83bull, 0xfffffe68aa112eccull, 0x0000001b2cc23d7bull, 0xfffffffe26684ff0ull, 0x000000002099487full, 0xfffffffffdd2b44aull, 0x00000000001c46c3ull },
	{ 0x733c54782737a104ull, 0xfe169a79cf86b92bull, 0x0016cb62517117f4ull, 0xfffec7d1e890afd5ull, 0x000011f9587f1cf0ull, 0xfffffeeb083ce233ull, 0x000000112aa0038cull, 0xfffffffeea083376ull, 0x0000000011ca4743ull, 0xfffffffffee3e6d3ull, 0x00000000000daa3full },
	{ 0x6f94d1bd4688bfdfull, 0xfe40cb90a21f52caull, 0x001384caef1e458eull, 0xffff061a7dfc635aull, 0x00000d705cdecae3ull, 0xffffff3ea086dfe9ull, 0x0000000b30ecb936ull, 0xffffffff56c922d1ull, 0x000000000a1e5d8full, 0xffffffffff686e13ull, 0x000000000006ee17ull },
	{ 0x6c3b981c824f3848ull, 0xfcca335ec2bdf560ull, 0x004383277ca35f16ull, 0xfff9a8c7772c8104ull, 0x0000a3cba3705e34ull, 0xffffeebaf14d24c6ull, 0x000001dfe1ac69eaull, 0xffffffcae62032a5ull, 0x00000005e33219b1ull, 0xffffffff64e54f0cull, 0x000000000ae43c5full },
	{ 0x664b7f254e766c6dull, 0xfd4076cf0d442fe3ull, 0x0033bfe53163a838ull, 0xfffbaa9a72f22038ull, 0x000063a6c061db79ull, 0xfffff6a72e778843ull, 0x000000e7050e5022ull, 0xffffffe940cca0c6ull, 0x00000002405b9fcfull, 0xffffffffca5a5499ull, 0x000000000380476cull },
	{ 0x612bf803377f7dfbull, 0xfd9c5b22d9b9fed1ull, 0x0028c4089615ca1bull, 0xfffceb1ba40966feull, 0x00003fdde104f32cull, 0xfffffa9abbd11624ull, 0x00000078161fd412ull, 0xfffffff559ba68f8ull, 0x00000000f3946767ull, 0xffffffffeb52014dull, 0x000000000142b2b7ull },
	{ 0x5cb0825f38e4a154ull, 0xfde58b9109231c20ull, 0x0020d4dbbb1f53e7ull, 0xfffdbcba93e297c3ull, 0x00002ab19205ca31ull, 0xfffffcb7f52a5fbbull, 0x000000426cecc1d9ull, 0xfffffffaa44e04b4ull, 0x000000006fa77a62ull, 0xfffffffff74aa067ull, 0x00000000007f7ab5ull },
	{ 0x58b90bfbe8e7bcd6ull, 0xfe2107428c79803dull, 0x001aede3660fc67full, 0xfffe4b3b35abc941ull, 0x00001d8cdb74a005ull, 0xfffffdea9de8e38eull, 0x00000026ae026536ull, 0xfffffffd237e7d88ull, 0x0000000036be86d9ull, 0xfffffffffc0d945bull, 0x0000000000366884ull },
	{ 0x552dc3f53513b0e1ull, 0xfe5231591e0dabe5ull, 0x00166e19eabbf537ull, 0xfffeaf4508b541f7ull, 0x0000150f30d68779ull, 0xfffffea0d93d5efbull, 0x0000001784390e5full, 0xfffffffe64c04fe4ull, 0x000000001c67db48ull, 0xfffffffffe18bb47ull, 0x000000000018cac0ull },
	{ 0x51fc88f1735ec312ull, 0xfe7b693fbce035afull, 0x0012edce8498f5bcull, 0xfffef76e05a98654ull, 0x00000f62dd9332feull, 0xffffff1195a3bcd4ull, 0x0000000ed5380ca0ull, 0xffffffff0f0d2856ull, 0x000000000f781291ull, 0xffffffffff0835f1ull, 0x00000000000bf2e5ull },
	{ 0x4f1742eecdb4e6efull, 0xfe9e645d15bc8d4cull, 0x001027e5ea2e7612ull, 0xffff2cb42218ca0full, 0x00000b7c21875ff1ull, 0xffffff59c5191605ull, 0x00000009a82c76bcull, 0xffffffff6d8a4b3cull, 0x0000000008c86b8full, 0xffffffffff7c12d0ull, 0x0000000000060bbdull },
	{ 0x4c72ca9b1b0005caull, 0xfd78ca59c35c3773ull, 0x0037b3ca2004e0ffull, 0xfffaa6a9dfc32461ull, 0x00008bbfa5208082ull, 0xfffff12c6deb53c5ull, 0x0000019d997fe3dfull, 0xffffffd219578c94ull, 0x000000051a2b1a30ull, 0xffffffff794838bbull, 0x000000000979ce8full },
	{ 0x47ca10f766e94bc4ull, 0xfdda135d1ad31721ull, 0x002a6ef79136151bull, 0xfffc5c642a9915caull, 0x000054bddeb1832bull, 0xfffff7fed8390605ull, 0x000000c6af402cc8ull, 0xffffffec61194e3dull, 0x00000001f250d6c0ull, 0xffffffffd186a354ull, 0x0000000003099015ull },
	{ 0x43cc694c3077ec89ull, 0xfe25359742c1f135ull, 0x00213ac477af0404ull, 0xfffd6c863d67443dull, 0x000036218305a0acull, 0xfffffb6409b0749eull, 0x0000006712d699f7ull, 0xfffffff6d4a1b5c5ull, 0x00000000d236384dull, 0xffffffffee1eec72ull, 0x0000000001175c5cull },
	{ 0x405486d88b5e2bb4ull, 0xfe60b45d6403d174ull, 0x001a9c66da248ef4ull, 0xfffe1de9d6365a56ull, 0x00002410734956e7ull, 0xfffffd342de9d59dull, 0x00000038e780fce2ull, 0xfffffffb64e15e19ull, 0x000000006035c43eull, 0xfffffffff87b91e3ull, 0x00000000006e2d51ull },
	{ 0x3d47a7274185b70full, 0xfe90ca673f2a0decull, 0x0015b58f3d4d6c2full, 0xfffe961917555ad6ull, 0x000018e0aa18d0ccull, 0xfffffe3aa668e340ull, 0x00000021121a9b92ull, 0xfffffffd8b53e5d5ull, 0x000000002f1b3358ull, 0xfffffffffc98f46bull, 0x00000000002ef5d3ull },
	{ 0x3a9201ba9cc60704ull, 0xfeb8532f802f6531ull, 0x0011fd2a322966feull, 0xfffeea2efc8498f5ull, 0x000011ab6167f7cfull, 0xfffffed6505b11ccull, 0x0000001410ef663aull, 0xfffffffe9f9aaaa6ull, 0x000000001868f8f6ull, 0xfffffffffe5c72feull, 0x0000000000155f96ull },
	{ 0x382497f4ecd09d58ull, 0xfed94d5d69abd4a0ull, 0x000f1b2d1da9e134ull, 0xffff26a1a1dd6d01ull, 0x00000cddb31781fbull, 0xffffff366b26ab6eull, 0x0000000ca1731657ull, 0xffffffff31dd9669ull, 0x000000000d46906aull, 0xffffffffff2ae3b9ull, 0x00000000000a4a53ull },
	{ 0x35f3ce8d3b4c143full, 0xfef52769f7964e0dull, 0x000cd51249cc6d18ull, 0xffff531bb25c9715ull, 0x0000099252bab6d1ull, 0xffffff73d3b28872ull, 0x0000000834b63678ull, 0xffffffff82e8bef8ull, 0x0000000007870b0eull, 0xffffffffff8ea9e6ull, 0x0000000000053397ull },
};

/*
 * S(z) = (lgamma(x) - (x - 1/2) log x + x - log(2 pi)/2) x for z = 1/x^2 <= 1/64, in w = 64 z: b0 in Q64, b1..b6 signed in
 * Q63 (within 2^-60.38 absolutely; divided by x from 8, below 2^-63)
 */
static const uint64_t stirling[7] = { 0x155555555555554aull, 0xfffe93e93e93eb59ull, 0x000001a01a017e2dull, 0xfffffffb1fb2c934ull, 0x000000001b92c63cull, 0xffffffffff0836a0ull, 0x00000000000a60e0ull };

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

/* a b as a 128-bit (hi, lo), from four 32 x 32 products */
static inline void lgamma_product(uint64_t a, uint64_t b, uint64_t *hi, uint64_t *lo)
{
	uint64_t p00 = __libc_trig_wide((uint32_t)a, (uint32_t)b);
	uint64_t p01 = __libc_trig_wide((uint32_t)a, (uint32_t)(b >> 32));
	uint64_t p10 = __libc_trig_wide((uint32_t)(a >> 32), (uint32_t)b);
	uint64_t middle = (p00 >> 32) + (uint32_t)p01 + (uint32_t)p10;

	*lo = (middle << 32) | (uint32_t)p00;
	*hi = __libc_trig_wide((uint32_t)(a >> 32), (uint32_t)(b >> 32)) + (p01 >> 32) + (p10 >> 32) + (middle >> 32);
}

/* The double of (hi 2^64 + lo) 2^e with sign, nonzero: its top 64 bits normalized, rounded once */
static inline double lgamma_wide(uint64_t sign, uint64_t hi, uint64_t lo, int32_t e)
{
	int k;

	if (hi == 0)
		return __libc_dp_fixed_normal(sign, e + 63, lo);
	k = __builtin_clzll(hi);
	if (k != 0)
		hi = (hi << k) | (lo >> (64 - k));

	return __libc_dp_fixed_double(sign, e + 127 - k, hi);
}

/* ln x in Q64 as (hi, lo), signed, for x = m 2^(e - 52): ln z by the fixed log kernel, z = m 2^-52 in [45/64, 90/64) (halved from 90/64, e one higher), plus e ln 2 */
static inline void lgamma_log(uint64_t m, int32_t e, uint64_t *hi, uint64_t *lo)
{
	constexpr uint64_t ln2 = 0xb17217f7d1cf79acull;
	uint64_t z = m << 11;
	int64_t lz;
	uint64_t ehi;
	uint64_t elo;

	if (z >= 0xb400000000000000ull) {
		z >>= 1;
		++e;
	}
	lz = __libc_dp_fixed_log_core(z);
	__libc_dp_fixed_k_product((uint32_t)(e < 0 ? -e : e), ln2, &ehi, &elo);
	if (e < 0) {
		ehi = ~ehi + (elo == 0);
		elo = -elo;
	}
	*lo = elo + (uint64_t)lz;
	*hi = ehi + (lz < 0 ? UINT64_MAX : 0) + (*lo < elo);
}

/*
 * The fast lgamma of the soft-double cores (PLAN D162, D219, D226), positive x in 64-bit fixed point with no
 * soft-double operation on the common path, within 1 ulp (docs/research/libm-fast-double.md, section 33):
 * - from 1/2 to 8, (x - 1)(x - 2) h(x): both factors exact integers in units of x's last place, their product exactly
 *   in 128 bits, so that the zeros 1 and 2 keep the relative accuracy (+0 there);
 * - below 1/2, -log x - x (1 - x) h(1 + x): 1 + x in Q63 (its truncation only picks the piece and moves u), the log by
 *   the fixed kernel plus e ln 2, in 128-bit Q64;
 * - from 8, Stirling: (x - 1/2)(log x - 1) as a 128-bit product of Q63 and Q58, plus log(2 pi)/2 - 1/2 + S(1/x^2)/x
 *   in Q64, 1/x by __libc_dp_fixed_reciprocal;
 * - negative x and the ends by the double-FPU file's paths in double (__libc_fast_lgamma_negative, _ends: the
 *   recurrence and the reflection, which keep 2^-60 near the negative zeros, D225).
 * These cores round to nearest and raise no flags (D44). C23 7.12.8.3.
 */
double lgamma(double x)
{
	uint64_t bits = __libc_double_bits(x);
	uint64_t magnitude = bits & 0x7fffffffffffffffull;
	int32_t e = (int32_t)(magnitude >> 52) - 1023;
	uint64_t m = (magnitude & 0x000fffffffffffffull) | 0x0010000000000000ull;
	uint64_t hi;
	uint64_t lo;
	uint64_t h;

	if (magnitude - 0x3c30000000000000ull >= 0x43b0000000000000ull - 0x3c30000000000000ull)
		return __libc_fast_lgamma_ends(x);
	if ((bits >> 63) != 0)
		return __libc_fast_lgamma_negative(x);

	if (e >= -1 && e < 3) {
		int64_t a = (int64_t)m - (int64_t)(1ull << (52 - e));
		int64_t b = (int64_t)m - (int64_t)(1ull << (53 - e));
		uint64_t sign = (a < 0) != (b < 0) ? 0x8000000000000000ull : 0;
		int32_t s = 0;
		uint64_t n;

		if (a == 0 || b == 0)
			return 0.0;
		/* |a b| 2^(2 e - 104), below 2^108: its top 64 bits n 2^s */
		lgamma_product((uint64_t)(a < 0 ? -a : a), (uint64_t)(b < 0 ? -b : b), &hi, &lo);
		n = lo;
		if (hi != 0) {
			int k = __builtin_clzll(hi);

			s = 64 - k;
			n = k != 0 ? (hi << k) | (lo >> s) : hi;
		}
		h = piece(pieces[(magnitude - 0x3fe0000000000000ull) >> 49], 10, magnitude << 15);
		lgamma_product(n, h, &hi, &lo);
		return lgamma_wide(sign, hi, lo, s + 2 * e - 168);
	}
	if (e < -1) {
		uint64_t y = 0x8000000000000000ull + (e >= -11 ? m << (11 + e) : m >> (-11 - e));
		uint64_t w = 0 - (e >= -12 ? m << (12 + e) : m >> (-12 - e));
		uint64_t t;

		h = piece(pieces[8 + ((y >> 60) & 7)], 10, y << 4);
		/* x (1 - x) h = t 2^(e - 63), in Q64 t >> (-1 - e) */
		t = __libc_trig_high64(__libc_trig_high64(m << 11, w), h) >> (-1 - e);
		lgamma_log(m, e, &hi, &lo);
		/* -log x - t, positive (at least 0.57) */
		hi = ~hi + (lo == 0);
		lo = -lo;
		hi -= lo < t;
		lo -= t;
		return lgamma_wide(0, hi, lo, -64);
	} else {
		uint64_t xm = (m << 11) - (1ull << (62 - e));
		uint64_t y = __libc_dp_fixed_reciprocal(m << 11);
		uint64_t c;
		uint64_t l;
		int32_t shift = 57 - e;

		/* log x - 1 in Q58 (log x below 42) */
		lgamma_log(m, e, &hi, &lo);
		l = ((hi << 58) | (lo >> 6)) - (1ull << 58);
		/* log(2 pi)/2 - 1/2 + S(1/x^2)/x in Q64: 1/x = y 2^(-64 - e), 64/x^2 in Q64 = high(y, y) >> (2 e - 6) */
		c = 0x6b3f8e4325f5a535ull + (__libc_trig_high64(piece(stirling, 6, __libc_trig_high64(y, y) >> (2 * e - 6)), y) >> e);
		/* (x - 1/2)(log x - 1) = xm l 2^(e - 121), and c in the same units */
		lgamma_product(xm, l, &hi, &lo);
		if (shift >= 0) {
			uint64_t add = c << shift;

			hi += (shift != 0 ? c >> (64 - shift) : 0) + (lo + add < lo);
			lo += add;
		} else {
			uint64_t add = c >> -shift;

			lo += add;
			hi += lo < add;
		}
		return lgamma_wide(0, hi, lo, e - 121);
	}
}
