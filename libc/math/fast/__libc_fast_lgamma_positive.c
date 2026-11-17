/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * __libc_fast_lgamma_positive.c
 *
 * Created on: Nov 17, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include <math.h>
#include <stdint.h>

#include "internal/math-fast.h"

/*
 * h(y) = lgamma(y)/((y - 1)(y - 2)) on [1, 8), smooth and positive (gamma at 1, 1 - gamma at 2), in 24 pieces of eight a
 * binade (widths 1/8, 1/4, 1/2), indexed by y's exponent and top three fraction bits: the centre m, then a polynomial in
 * t = y - m of degree 11 fitted here (Chebyshev, 200-bit arithmetic; within 2^-62.48 of h relatively, the coefficients'
 * rounding included), c0 and c1 as pairs, then c2..c11
 */
static const double pieces[24][15] = {
	{ 0x1.1000000000000p+0, 0x1.1ffbc90ebe138p-1, -0x1.6776095315a27p-55, -0x1.d112f2b3b57e6p-3, -0x1.e5b823677d723p-58, 0x1.162eda95b8ef2p-3, -0x1.84577cc33c942p-4, 0x1.2502bc32fb55bp-4, -0x1.ceacb08b24b01p-5, 0x1.781ead4734311p-5, -0x1.3808ac04c0a0ep-5, 0x1.06c51bb61ed97p-5, -0x1.bfbbbf1a6c616p-6, 0x1.846f200d4bc50p-6, -0x1.50973cb0919b9p-6 },
	{ 0x1.3000000000000p+0, 0x1.1273305f1abb7p-1, -0x1.b97bfb8aff600p-56, -0x1.939c7d39be5f8p-3, -0x1.96b02fa6cd772p-57, 0x1.b24a56b39d3a2p-4, -0x1.0fc6c5ac9214fp-4, 0x1.6f6f1e7d40c23p-5, -0x1.03e2194284ff0p-5, 0x1.7a89e35c2ee65p-6, -0x1.19584e6e967f4p-6, 0x1.a881302ea24b8p-7, -0x1.43f6ff1f27eb4p-7, 0x1.f698718452970p-8, -0x1.860a9c815b575p-8 },
	{ 0x1.5000000000000p+0, 0x1.069fc6dbf31d1p-1, -0x1.914f9e75ec07ep-55, -0x1.630c70df1f666p-3, -0x1.3c068db7fc8edp-57, 0x1.5b5dc5be08dc4p-4, -0x1.8a17bf609c0f9p-5, 0x1.e297bf53a02cap-6, -0x1.3521fac486da7p-6, 0x1.97cf2a4f7cd96p-7, -0x1.1285fe058b57dp-7, 0x1.7728e4d4be864p-8, -0x1.034d437e31993p-8, 0x1.6bd7eac3155b8p-9, -0x1.ff61cf061bb97p-10 },
	{ 0x1.7000000000000p+0, 0x1.f85341d79e780p-2, -0x1.25e621a1671b8p-57, -0x1.3bd15648e7adep-3, 0x1.88497d44d5831p-57, 0x1.1b7132981eef3p-4, -0x1.2620eb41611a2p-5, 0x1.492d5f927c092p-6, -0x1.815c1a5009255p-7, 0x1.d08c15f73b892p-8, -0x1.1dc5ee2e25a20p-8, 0x1.64e450dc93cebp-9, -0x1.c2d85aca78d43p-10, 0x1.20c38743fc2ecp-10, -0x1.72daaa443066cp-11 },
	{ 0x1.9000000000000p+0, 0x1.e5a06e5dfae2ap-2, -0x1.06505071e75adp-56, -0x1.1b8a2e50cbd00p-3, 0x1.6874fe2352d39p-57, 0x1.d6539c27ad250p-5, -0x1.c1c7d1612a428p-6, 0x1.cf827ff11eb1ep-7, -0x1.f38bffdf0ff2fp-8, 0x1.1532dab7022f1p-8, -0x1.39fcbffcdbec2p-9, 0x1.6907220bf9eadp-10, -0x1.a3e694ec58343p-11, 0x1.eeda9ecdd42b1p-12, -0x1.248c8e1f89478p-12 },
	{ 0x1.b000000000000p+0, 0x1.d4c5c0303a8f4p-2, 0x1.97178ccfded4cp-56, -0x1.009230d25df4fp-3, -0x1.99a68e412b82ap-57, 0x1.8bc0885a5b11cp-5, -0x1.5f029f824284cp-6, 0x1.4f3372a6cbd72p-7, -0x1.4eb2197745339p-8, 0x1.581f7d689ac81p-9, -0x1.6922673497416p-10, 0x1.80b8fe3b31f6fp-11, -0x1.9e95287661db1p-12, 0x1.c46c34f5a6962p-13, -0x1.ef9d0ff471474p-14 },
	{ 0x1.d000000000000p+0, 0x1.c5782317e9399p-2, 0x1.3aacd9c610f55p-60, -0x1.d37b9d371f43bp-4, 0x1.b4c87a5622293p-58, 0x1.510c8665b49fap-5, -0x1.16c7299cfc460p-6, 0x1.f01acecd84856p-8, -0x1.cd6e036f0ebadp-9, 0x1.b9ea1f9464ec3p-10, -0x1.affd7d6fe6380p-11, 0x1.acb227fe548d1p-12, -0x1.ae59179438685p-13, 0x1.b547e75229ccfp-14, -0x1.be3ccb49ec087p-15 },
	{ 0x1.f000000000000p+0, 0x1.b77c8b7131432p-2, 0x1.2e76a20699813p-57, -0x1.ac6492c8647adp-4, -0x1.4a4e9643bd2d9p-59, 0x1.2213393256f06p-5, -0x1.c19b3bb0b130dp-7, 0x1.76856bce2f7bcp-8, -0x1.460562edf824ap-9, 0x1.2434975605e6fp-10, -0x1.0b534e42ecfd0p-11, 0x1.f090da04e3c9ep-13, -0x1.d28aaaf83d7c2p-14, 0x1.bb85bf989197ap-15, -0x1.a79d674aa1ac8p-16 },
	{ 0x1.1000000000000p+1, 0x1.a4982ad0ea0dfp-2, -0x1.7dede454aae6cp-57, -0x1.7b5f07b2d5bfbp-4, -0x1.f720e0a608d3bp-59, 0x1.d7107a33b343fp-6, -0x1.4da233b884c19p-7, 0x1.fb56fb0e8a7b0p-9, -0x1.92ed59a0053edp-10, 0x1.496fd7254d053p-11, -0x1.12eee24d59673p-12, 0x1.d1e0d26924c96p-14, -0x1.8f52cb9960925p-15, 0x1.5c877ed760165p-16, -0x1.2fba78726afedp-17 },
	{ 0x1.3000000000000p+1, 0x1.8e9331cef6549p-2, -0x1.adaa8e68fa86ap-57, -0x1.476d23ab17672p-4, 0x1.3888d777d1ecdp-58, 0x1.6e53146b4e57dp-6, -0x1.d1b1501e6aa4bp-8, 0x1.3d42534c3a806p-9, -0x1.c33da3aad198bp-11, 0x1.4a4a080667d34p-12, -0x1.ed8457af6d293p-14, 0x1.76554012194a5p-15, -0x1.1f416ae58bb25p-16, 0x1.c023ab74ca6fbp-18, -0x1.5da84444f1fdfp-19 },
	{ 0x1.5000000000000p+1, 0x1.7b6fde69143dcp-2, -0x1.8aee76ee153edp-59, -0x1.1e891590bcc40p-4, -0x1.70fcf4da8ccf9p-60, 0x1.23f152284c7dfp-6, -0x1.50c7a43e65b8cp-8, 0x1.9fc273aca7689p-10, -0x1.0bb6a698022e6p-11, 0x1.62c28e3a2917bp-13, -0x1.dfcb96a503e58p-15, 0x1.49678f98b19c6p-16, -0x1.c99d5bfea34fap-18, 0x1.42b2b1b97d97ap-19, -0x1.c7d7597b15889p-21 },
	{ 0x1.7000000000000p+1, 0x1.6a97b2c1d3cb9p-2, 0x1.baa4a4d004b11p-61, -0x1.fb3da9221ff99p-5, -0x1.3f0769bee7414p-61, 0x1.dac1ed6deb28ap-7, -0x1.f57b52fecc171p-9, 0x1.1b07308df568ep-10, -0x1.4d0c44d130ae8p-12, 0x1.9325574e0aa0ep-14, -0x1.f1ff00eda040cp-16, 0x1.3845c2a4a5f2ep-17, -0x1.8c3a2f7a64af8p-19, 0x1.fdeacf9ad737cp-21, -0x1.48f7658c14ec9p-22 },
	{ 0x1.9000000000000p+1, 0x1.5b9c82419623ap-2, -0x1.70f9a736f7c3dp-59, -0x1.c54446f2570fdp-5, 0x1.658c9f5e14a0dp-60, 0x1.888973e717f36p-7, -0x1.7e85bd27ce602p-9, 0x1.8dcceb6c6b136p-11, -0x1.aefbeaaf0f5c3p-13, 0x1.e02cb7dfcb8a1p-15, -0x1.10eee0e97a1bdp-16, 0x1.3afdd1ec64865p-18, -0x1.6fcd347e7f842p-20, 0x1.b3441b0df1d9ep-22, -0x1.026bf3df63300p-23 },
	{ 0x1.b000000000000p+1, 0x1.4e2b6ae4d11c3p-2, 0x1.47a6888955d77p-56, -0x1.9852fb16da685p-5, -0x1.af268f6b65135p-63, 0x1.492f65a96cc3ep-7, -0x1.29d300437bb3ep-9, 0x1.1f2eda8625cdcp-11, -0x1.205088cbd8649p-13, 0x1.298fa3878dd6bp-15, -0x1.394e1733cf437p-17, 0x1.4ee0efa45ab72p-19, -0x1.6a230ef272400p-21, 0x1.8cac024536aa2p-23, -0x1.b43b4eba8d7d3p-25 },
	{ 0x1.d000000000000p+1, 0x1.4204a3477019bp-2, -0x1.446c08096899cp-56, -0x1.7267f59e2bf1cp-5, 0x1.8d0fee85eaaeep-62, 0x1.176eb24c6d70dp-7, -0x1.d7fa528b71f4cp-10, 0x1.a852a3a4b0083p-12, -0x1.8ced4f84998d8p-14, 0x1.7d9434c4688dfp-16, -0x1.762b1c57a59f5p-18, 0x1.746e3b4e6ea5ap-20, -0x1.7709fac36006fp-22, 0x1.7e5c9554e9e33p-24, -0x1.8791eb20f71bep-26 },
	{ 0x1.f000000000000p+1, 0x1.36f61f453267fp-2, -0x1.f230a80729b8ap-62, -0x1.520caa081b662p-5, -0x1.50123d1baaf98p-60, 0x1.df6ce6e7c4b61p-8, -0x1.7bb876a9a3836p-10, 0x1.3fcd146291a83p-12, -0x1.18135faf1a733p-14, 0x1.f7fc63a5024f7p-17, -0x1.ce72f43300670p-19, 0x1.aeb1129ee0ef9p-21, -0x1.95cb0f9cd435fp-23, 0x1.82ef5839d8418p-25, -0x1.72bf11c4a67c6p-27 },
	{ 0x1.1000000000000p+2, 0x1.2818a46d82d7dp-2, 0x1.8e104449382c1p-57, -0x1.29a1ab159fcb3p-5, 0x1.03a71cb4c7ff5p-60, 0x1.83723d441b7d3p-8, -0x1.18ce798906bebp-10, 0x1.b02516303cce2p-13, -0x1.597c73d655b44p-15, 0x1.1b9f3511ea5a5p-17, -0x1.dac7c7c8cbd98p-20, 0x1.934536be2880bp-22, -0x1.5a824c8aac34dp-24, 0x1.2f39c669ab3a9p-26, -0x1.08f937e6c02c8p-28 },
	{ 0x1.3000000000000p+2, 0x1.16e1f41ba4362p-2, -0x1.3a1e4bc853792p-57, -0x1.fe105e680ca9dp-6, -0x1.0a00850244cf6p-61, 0x1.2b7abaf75f8cep-8, -0x1.862d01833a657p-11, 0x1.0d58316a7f6c8p-13, -0x1.81f486adbe75fp-16, 0x1.1bc663a7cf853p-18, -0x1.a94ca6fbdf40ep-21, 0x1.435a4ab827065p-23, -0x1.f154af40bf109p-26, 0x1.84c80151dc36dp-28, -0x1.30078c9037e38p-30 },
	{ 0x1.5000000000000p+2, 0x1.08067793138d0p-2, -0x1.1faadbf523eafp-56, -0x1.bb648d33a5c76p-6, 0x1.e50842c69c902p-62, 0x1.da95818c3c326p-9, -0x1.18e6d5565f1a2p-11, 0x1.5fcac81509852p-14, -0x1.c8d319a26d63cp-17, 0x1.30347dbc82e0fp-19, -0x1.9cc53bbbafeaep-22, 0x1.1c105eb01b315p-24, -0x1.8b69148de798cp-27, 0x1.175c572da7bafp-29, -0x1.8b5fea6fa9df9p-32 },
	{ 0x1.7000000000000p+2, 0x1.f610a9d6c6927p-3, 0x1.fd0f4a7e0962bp-57, -0x1.860799a8fe6cap-6, -0x1.2e59591220413p-63, 0x1.7fc11fd12db82p-9, -0x1.a06e02175c51ap-12, 0x1.dd5ac4aea9682p-15, -0x1.1b70255dbfa45p-17, 0x1.59062d93ee779p-20, -0x1.abbe27e668d1cp-23, 0x1.0ce54a2c6b9f5p-25, -0x1.55db4c33307cfp-28, 0x1.b8b53f12c98f2p-31, -0x1.1cca0750f1811p-33 },
	{ 0x1.9000000000000p+2, 0x1.df17a6e91f743p-3, -0x1.412a34f1807c3p-57, -0x1.5a838e1785297p-6, 0x1.85d0e8b487081p-62, 0x1.3b9eca78cf6cap-9, -0x1.3c4558445f65bp-12, 0x1.4e550dd377719p-15, -0x1.6dd76a068ef86p-18, 0x1.9a22d083941e4p-21, -0x1.d41ee73c15a1ep-24, 0x1.0ede66122f35fp-26, -0x1.3cec1326e91b2p-29, 0x1.77ae97e3daa3cp-32, -0x1.bec4747bd8176p-35 },
	{ 0x1.b000000000000p+2, 0x1.ca9881730c98ap-3, 0x1.f33107a373c4ap-57, -0x1.367786c859475p-6, -0x1.74d8c96206361p-61, 0x1.0758f9657b548p-9, -0x1.ea664f53c4569p-13, 0x1.e11915eebbb14p-16, -0x1.e8309717022f2p-19, 0x1.fb49d906be31fp-22, -0x1.0c410d863d49ep-24, 0x1.1f971c2603ff8p-27, -0x1.37ac27fb95593p-30, 0x1.55f95792a0d8dp-33, -0x1.789f629188ed5p-36 },
	{ 0x1.d000000000000p+2, 0x1.b829f10666572p-3, 0x1.fda33affedd25p-57, -0x1.18344a1bb572cp-6, 0x1.3b4314bc46e8bp-60, 0x1.bcef6f16757d0p-10, -0x1.82fa0a6208e50p-13, 0x1.6239b01216c57p-16, -0x1.4f2807f66fa0dp-19, 0x1.44963fc7ff70fp-22, -0x1.3fd7d135b0c66p-25, 0x1.3f6aa9b657ad7p-28, -0x1.426925104e3acp-31, 0x1.494a5c2bc72a1p-34, -0x1.51b504b5102e7p-37 },
	{ 0x1.f000000000000p+2, 0x1.a779b3885d919p-3, 0x1.12ee83ed0cdb5p-61, -0x1.fd01204154ff7p-7, -0x1.4703b89541559p-63, 0x1.7bedef91c7726p-10, -0x1.3615305d1340ep-13, 0x1.0a14b40748cc0p-16, -0x1.d7b61ca9f954dp-20, 0x1.abcd174fc987fp-23, -0x1.8aa48e95e57dep-26, 0x1.70e2fd4c2bb14p-29, -0x1.5c7507fdd788ap-32, 0x1.4ce5a0bcce3bdp-35, -0x1.3f71bf5b3d963p-38 },
};

/* h at t from its piece's centre as hi + *lo: c0 + c1 t exactly (two fused multiply-adds, or an exact product and a fast two-sum), the rest in plain double */
[[gnu::always_inline]] static inline double lgamma_h(const double *c, double t, double *lo)
{
	double q = c[4] + t * (c[5] + t * (c[6] + t * (c[7] + t * (c[8] + t * (c[9] + t * (c[10] + t * (c[11] + t * (c[12] + t * (c[13] + t * c[14])))))))));
	struct __libc_dd product;
	struct __libc_dd sum;
	double r;

	if (__libc_fpu_fma) {
		r = __builtin_fma(c[3], t, c[1]);
		*lo = __builtin_fma(c[3], t, c[1] - r) + (c[2] + t * q);
		return r;
	}
	product = __libc_fast_product(c[3], t);
	sum = __libc_dd_fast_sum(c[1], product.hi);
	*lo = sum.lo + product.lo + (c[2] + t * q);

	return sum.hi;
}

/* The piece of y in [1, 8): its exponent and top three fraction bits */
static inline const double *lgamma_piece(double y)
{
	return pieces[(__libc_double_bits(y) - 0x3ff0000000000000ull) >> 49];
}

/* (p_hi + p_lo)(h_hi + h_lo) as hi + *lo, p_hi h_hi exactly */
static inline double lgamma_scaled(double p_hi, double p_lo, double h_hi, double h_lo, double *lo)
{
	struct __libc_dd product = __libc_fast_product(p_hi, h_hi);

	*lo = product.lo + (p_hi * h_lo + p_lo * h_hi);
	return product.hi;
}

/*
 * lgamma(x) for 2^-60 <= x < 2^60 as hi + *lo, the fast lgamma's positive path in double (PLAN D225; docs/research/libm-fast-double.md, section 32):
 * - below 1, x (x - 1) h(1 + x) - log x: h's piece from 1 + x (rounded only to pick it), t = x - (m - 1), x - 1 as an
 *   exact pair, log x as a pair (__libc_fast_log_pair, relative however close x is to 1), and |log x| >= |lgamma(1 + x)|;
 * - below 8, (x - 1)(x - 2) h(x), both factors exact and their product an exact pair, so that the zeros 1 and 2 keep
 *   the relative accuracy (+0 there);
 * - from 8, Stirling: (x - 1/2)(log x - 1) + log(2 pi)/2 - 1/2 + S(1/x^2)/x, x - 1/2 with its rounding error (x from
 *   2^52), log x - 1 exact from log's pair, S of degree 6 fitted here (within 2^-57.59 absolutely, below 2^-60 once
 *   divided by x).
 */
double __libc_fast_lgamma_positive(double x, double *lo)
{
	struct __libc_dd sum;
	struct __libc_dd p;
	const double *c;
	double h_lo;
	double h;
	double l_lo;
	double l;
	double r_lo;
	double r;

	if (x < 1.0) {
		c = lgamma_piece(1.0 + x);
		h = lgamma_h(c, x - (c[0] - 1.0), &h_lo);
		sum = __libc_dd_sum(x, -1.0);
		p = __libc_fast_product(x, sum.hi);
		r = lgamma_scaled(p.hi, p.lo + x * sum.lo, h, h_lo, &r_lo);
		l = __libc_fast_log_pair(x, 0.0, 0, &l_lo);
		sum = __libc_dd_fast_sum(-l, r);
		*lo = sum.lo + (r_lo - l_lo);
		return sum.hi;
	}
	if (x < 8.0) {
		/* The zeros first, so that they raise no inexact (F.10.5.3) */
		p = __libc_fast_product(x - 1.0, x - 2.0);
		if (p.hi == 0) {
			*lo = 0.0;
			return 0.0;
		}
		c = lgamma_piece(x);
		h = lgamma_h(c, x - c[0], &h_lo);
		return lgamma_scaled(p.hi, p.lo, h, h_lo, lo);
	} else {
		double y = x - 0.5;
		double y_lo = (x - y) - 0.5;
		double v = 1.0 / x;
		double z = v * v;

		l = __libc_fast_log_pair(x, 0.0, 0, &l_lo) - 1.0;
		p = __libc_fast_product(y, l);
		sum = __libc_dd_fast_sum(p.hi, 0x1.acfe390c97d69p-2);
		*lo = sum.lo + (p.lo + y * l_lo + y_lo * l + 0x1.3494bc9001442p-56 + v * (0x1.5555555555555p-4 + z * (-0x1.6c16c16c149f4p-9 + z * (0x1.a01a017dcf9e1p-11 + z * (-0x1.38134d4f8896fp-11 + z * (0x1.b92c33f9445e5p-11 + z * (-0x1.ef8d748aeb0f2p-10 + z * 0x1.4bffc9d8b455bp-8)))))));
		return sum.hi;
	}
}
