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

#include "internal/math-fast.h"

/*
 * erf on [m - 1/8, m + 1/8] for m = 1/2 + i/4, i = 0..22, as a polynomial in t = |x| - m of degree 11, fitted here
 * by least squares on Chebyshev nodes (within 2^-59.24 of erf, the rounding of the coefficients included): c0 and c1 as
 * pairs (hi, lo), then c2..c11
 */
static const double pieces[23][14] = {
	{ 0x1.0a7ef5c18edd2p-1, 0x1.44e5a451f1bb4p-56, 0x1.c1efca49a5011p-1, 0x1.70124c5802f80p-55, -0x1.c1efca49a4f9ep-2, -0x1.2bf531866e108p-3, 0x1.76f27de7df7afp-3, 0x1.dfeeb5a7d4d36p-8, -0x1.99f13a734d232p-5, 0x1.623c5e1d4a20cp-8, 0x1.493bedf81bbe1p-7, -0x1.1c1373ed46496p-9, -0x1.99074d6c16202p-10, 0x1.027e973fc7f7dp-11 },
	{ 0x1.6c1c9759d0e5fp-1, 0x1.b825e922ff3c9p-55, 0x1.492e42d78d2c5p-1, -0x1.6bb97dd1a3206p-55, -0x1.edc5644353c65p-2, 0x1.b6e8591f66756p-6, 0x1.349b5eaa2af59p-3, -0x1.b42a18905ddc2p-5, -0x1.b8477a27c1063p-6, 0x1.2e0afa4afd97ap-6, 0x1.2db61c6177bdfp-9, -0x1.040f1b74e2ef8p-8, 0x1.7bc38365e3c57p-13, 0x1.4522f254a7991p-11 },
	{ 0x1.af767a741088bp-1, -0x1.af6f037e1d1ffp-56, 0x1.a911f096fbc26p-2, -0x1.181beab1bc747p-56, -0x1.a911f096fbc9bp-2, 0x1.1b614b0f52850p-3, 0x1.1b614b0fa8106p-4, -0x1.1b614b0f6040fp-4, 0x1.2e459fb0fa720p-8, 0x1.f096fdce8a656p-7, -0x1.390e860dfdcc4p-8, -0x1.ee3214988bb3fp-10, 0x1.3f03a6e01c0a6p-10, 0x1.6aa9d77249ed8p-14 },
	{ 0x1.d8865d98abe01p-1, -0x1.f974f69532305p-55, 0x1.e4652fadcb6b2p-3, -0x1.f47b760185eb4p-59, -0x1.2ebf3dcc9f24ep-2, 0x1.571d01c5c5764p-3, -0x1.93a9a7ba12f93p-8, -0x1.8281ce0b87bbbp-5, 0x1.5d00034e6be4ep-6, 0x1.db43cde1bc289p-9, -0x1.756671de0bd74p-8, 0x1.cc1aeffb51504p-11, 0x1.9ebaf4af2e756p-11, -0x1.50bb32f85d32ap-12 },
	{ 0x1.eea5557137ae0p-1, -0x1.3d69b9645b004p-55, 0x1.e723726b824a9p-4, -0x1.9c9b2a718db81p-59, -0x1.6d5a95d0a1b24p-3, 0x1.1c2a02beb6aedp-3, -0x1.6d5a95d0e3f8cp-5, -0x1.e723726bed8c7p-7, 0x1.3ca3d7b9b27afp-6, -0x1.36d739aa01cb4p-8, -0x1.35b0702d6980ap-9, 0x1.c036d9dba6c29p-10, -0x1.7684a59c1279fp-14, -0x1.09d9f5fb45e20p-12 },
	{ 0x1.f92d077f8d56dp-1, 0x1.825d962220e3dp-56, 0x1.b055303221016p-5, -0x1.bcc292968fa02p-59, -0x1.7a4a8a2bdcd71p-4, 0x1.7148c3d57c2aap-4, -0x1.8a0da54340f67p-5, 0x1.b22257ddd28a9p-8, 0x1.25b379c46fcf7p-7, -0x1.8d10fc1f16e8dp-8, 0x1.7eba3ed29e1c4p-11, 0x1.d4d16209fe392p-11, -0x1.cce391713c44dp-12, -0x1.a97aa414ed242p-18 },
	{ 0x1.fd9ae142795e3p-1, 0x1.9671d277303a7p-56, 0x1.529b9e8cf9a1ep-6, 0x1.b6f2ee17d9c0ap-60, -0x1.529b9e8cf9a05p-5, 0x1.8b0ae3a478862p-5, -0x1.1a2c59757f5a4p-5, 0x1.ace7404c8bb6ep-7, 0x1.e193612116e02p-12, -0x1.bae0ace8c2b6cp-9, 0x1.a113e806ba608p-10, -0x1.a449d41ff10a6p-15, -0x1.134e357c1df75p-12, 0x1.afc58f7ee3a8dp-14 },
	{ 0x1.ff404760319b4p-1, 0x1.f4357fec6de65p-56, 0x1.d4143a9dfe965p-8, -0x1.b135ec49d3106p-64, -0x1.074b60f8df41dp-6, 0x1.63ef61e824404p-6, -0x1.38a98327624dcp-6, 0x1.5d3b17bbe925ep-7, -0x1.7cae0e9087ea8p-9, -0x1.5f83142741ebap-11, 0x1.0604227e2860dp-10, -0x1.83526fc5c68afp-12, -0x1.a447f1f105a97p-17, 0x1.10e80137d320cp-14 },
	{ 0x1.ffcaa8f4c9beap-1, 0x1.b1baf1c9b4899p-55, 0x1.1d83170fbf6fbp-9, -0x1.1908fa6008fe6p-67, -0x1.64e3dcd3af6cep-8, 0x1.119da0c46cd8fp-7, -0x1.1a89b97cba5cap-7, 0x1.90e81283c31c1p-8, -0x1.6ecdc0365c669p-9, 0x1.1c610d48cf134p-11, 0x1.11583ac9d6072p-12, -0x1.0672b6aad97eep-12, 0x1.47bfe39f44840p-14, 0x1.68a7e0b7f3763p-18 },
	{ 0x1.fff2cfb0453d9p-1, 0x1.9aa3f5c0bb48ep-55, 0x1.3360ccd23db39p-11, -0x1.017f550a7f249p-66, -0x1.a6a519a114e18p-10, 0x1.69cf466cce163p-9, -0x1.ab0c273ab30dep-9, 0x1.6935960624ba2p-9, -0x1.b2755c1e169aep-10, 0x1.52b6271add867p-11, -0x1.755677a10bbccp-14, -0x1.2abe196d56ed4p-14, 0x1.ccea9935e6388p-15, -0x1.078671fd2f6bcp-16 },
	{ 0x1.fffd1ac4135f9p-1, 0x1.ee7d2bdfdba96p-55, 0x1.2408e9ba3327ep-13, 0x1.9b00a65124a37p-67, -0x1.b60d5e974c4a5p-12, 0x1.9db74b1d1dd43p-11, -0x1.11c85b1ee2c9ep-10, 0x1.0a7b5546aecb8p-10, -0x1.82f234ff882ddp-11, 0x1.998b47dbce103p-12, -0x1.1aa73767fdbfdp-13, 0x1.d2a2204d7570bp-17, 0x1.085eae9d3090dp-16, -0x1.69f4614255f50p-17 },
	{ 0x1.ffff6f9f67e55p-1, 0x1.e1c45489a6294p-55, 0x1.e9b5e8d00ce7ep-16, -0x1.94c97c6bcd868p-74, -0x1.8de3cd29095c7p-14, 0x1.9aa489e3ca6a1p-13, -0x1.2c7d5ef1259a1p-12, 0x1.490a4d2376a7ep-12, -0x1.1454640a7ef85p-12, 0x1.647f7169bf04ap-13, -0x1.56762921fc027p-14, 0x1.b2c572ea96914p-16, -0x1.2c97ed159696bp-19, -0x1.8429807673c33p-19 },
	{ 0x1.ffffe710d565ep-1, 0x1.c9e2ccc4f8733p-55, 0x1.6a597219a93eap-18, 0x1.089b5fc27a318p-76, -0x1.3d0e43d673072p-16, 0x1.62ccea63ca25dp-15, -0x1.1c07721b8d5aap-14, 0x1.586bafcaa0087p-14, -0x1.46153ee725a7fp-14, 0x1.e827f96f83c00p-15, -0x1.1f63cf89ba7eep-15, 0x1.01366f33fa1f1p-16, -0x1.360c81735ee3dp-18, 0x1.cd09d9dfe8271p-22 },
	{ 0x1.fffffc2f171e3p-1, 0x1.85ef4a1ee12c8p-55, 0x1.d9371e2ff7c4fp-21, -0x1.9530615e73f2dp-75, -0x1.bba3ac4cf9f09p-19, 0x1.0b6a7b0f1aa71p-17, -0x1.d06f585f5d69dp-17, 0x1.3436bca075af4p-16, -0x1.4357b5fa455f9p-16, 0x1.110de4043b1c6p-16, -0x1.75661ea1de651p-17, 0x1.99f6c1fe01ffbp-18, -0x1.5df82e145bc06p-19, 0x1.9eafcc11e346dp-21 },
	{ 0x1.ffffff7b91176p-1, 0x1.0b2c6b0f7a866p-56, 0x1.10b1488aeb217p-23, 0x1.ccd8406795ab8p-80, -0x1.10b1488af4304p-21, 0x1.603a5308c6b0ap-20, -0x1.4980e24bed2fep-19, 0x1.da5f10dab06c2p-19, -0x1.105056fc11f15p-18, 0x1.fd7c66d524648p-19, -0x1.88c44a16f8ce6p-19, 0x1.f4233963ed179p-20, -0x1.05f7f1bb60f98p-20, 0x1.af45fe6b06ed9p-22 },
	{ 0x1.fffffff01a8b6p-1, 0x1.235527b3d8bbdp-60, 0x1.155a09065d408p-26, 0x1.fb129e55a6020p-83, -0x1.26afa996e4ffcp-24, 0x1.95ea6fe008499p-23, -0x1.96ba734e10b61p-22, 0x1.3b46801335bf1p-21, -0x1.8868eefc407b6p-21, 0x1.916e97c91553ap-21, -0x1.5668a984ba58ap-21, 0x1.eab03cddc7a61p-22, -0x1.2a2a3701c27bdp-22, 0x1.29b746ee9f8adp-23 },
	{ 0x1.fffffffe4fa30p-1, 0x1.d167eb7cd69c1p-57, 0x1.f1e3523b41750p-30, 0x1.499df0e9566c3p-84, -0x1.180fde41aa2d5p-27, 0x1.99b86656440ebp-26, -0x1.b598cb0803fc7p-25, 0x1.6b1baf2fd0b4bp-24, -0x1.e651045c1528fp-24, 0x1.0d6798c72b027p-23, -0x1.f5e329ba79205p-24, 0x1.8d26adb291f76p-24, -0x1.0fc429e0d7264p-24, 0x1.38036acf86081p-25 },
	{ 0x1.ffffffffd759dp-1, 0x1.f7befa06b05b3p-55, 0x1.8a61745ec6f1cp-33, -0x1.063aac29a3326p-87, -0x1.d453ba31d322ap-31, 0x1.6a8aeba4d8603p-29, -0x1.9b017a45302e4p-28, 0x1.6b43c92203925p-27, -0x1.042f4a0ca4095p-26, 0x1.35dc9816b9b9dp-26, -0x1.3825a60e2ff4cp-26, 0x1.0d443724adb9ap-26, -0x1.976d64240264fp-27, 0x1.05627cbfbb92dp-27 },
	{ 0x1.fffffffffc9e8p-1, -0x1.a759f0866f141p-56, 0x1.13af4f04f8149p-36, -0x1.6adde7b020898p-90, -0x1.589b22c82a819p-34, 0x1.196da0ab1394bp-32, -0x1.516d3c01c4290p-31, 0x1.3c51d0559e3ffp-30, -0x1.e235e7b154733p-30, 0x1.32c74d719c524p-29, -0x1.4bb755b5de4ecp-29, 0x1.34f6d4c8bb088p-29, -0x1.fe6790cbc7308p-30, 0x1.6786c0247cd63p-30 },
	{ 0x1.ffffffffffc05p-1, 0x1.07ba9731cf6cbp-55, 0x1.5422ef5d8510ap-40, -0x1.acd85e59804e8p-96, -0x1.be6dda2fa7a2fp-38, 0x1.7f8a0f3ffc532p-36, -0x1.e4cb492278545p-35, 0x1.e044b2fecccfep-34, -0x1.83eac51ce2827p-33, 0x1.06598b47ae8fep-32, -0x1.2ea8114d8c39ep-32, 0x1.2e2351b76b4b6p-32, -0x1.0dfb90c61a290p-32, 0x1.9c8c1dd797208p-33 },
	{ 0x1.fffffffffffbep-1, -0x1.182b3258f0bb8p-55, 0x1.7258610b31805p-44, 0x1.57f3316251044p-99, -0x1.fd398579abefbp-42, 0x1.cb12e2fa22bbdp-40, -0x1.31011cb9c10b4p-38, 0x1.3e4a1e7be2416p-37, -0x1.0f6f08dbccc0bp-36, 0x1.84a5546183660p-36, -0x1.dbfb9770be12dp-36, 0x1.fa4a9c9483547p-36, -0x1.e600821c9a1b7p-36, 0x1.8f284f94eac45p-36 },
	{ 0x1.ffffffffffffcp-1, 0x1.8115fd1f14da9p-56, 0x1.63daf8b49f8e4p-48, -0x1.e1c78ea381705p-102, -0x1.ff8ac595c787ap-46, 0x1.e2d06d77d700ep-44, -0x1.505d91ecf1560p-42, 0x1.70b6ff15ec1c4p-41, -0x1.4aee480cbf225p-40, 0x1.f3c66df1e821dp-40, -0x1.438b491b845b8p-39, 0x1.6ceb4eb40205fp-39, -0x1.7638867ea2c36p-39, 0x1.481e29ba1f408p-39 },
	{ 0x1.0000000000000p+0, -0x1.8cf81557720f2p-56, 0x1.2dc1190939d44p-52, -0x1.9f89ee86e29e2p-106, -0x1.c4a1a5a8fcb45p-50, 0x1.be584a6aa45a0p-48, -0x1.45542a1471458p-46, 0x1.75a818cc6f47ap-45, -0x1.5ff923765ad50p-44, 0x1.1772b9684c55bp-43, -0x1.7d248e046581bp-43, 0x1.c5fcf59b8f035p-43, -0x1.ef1dd275df1a6p-43, 0x1.cce61dd052747p-43 }
};

/*
 * (erf(x)/x - 2/sqrt(pi))/x^2 = p0 + z Q(z) for z = x^2 <= 1/4, p0 = -2/(3 sqrt(pi)) as a pair and Q of nine terms
 * fitted here (x^5 Q within 2^-61.9 of the rest of erf, relatively); p0 rounded alone left 2^-58.2
 */
static inline double erf_series(double z)
{
	return 0x1.ce2f21a042be2p-4 + z * (-0x1.b82ce31288b4bp-6 + z * (0x1.565bcd0e69b0dp-8 + z * (-0x1.c02db3ff6f002p-11 + z * (0x1.f9a32672932d0p-14 + z * (-0x1.f4d22b461bfb6p-17 + z * (0x1.b9dc5927a3862p-20 + z * (-0x1.5e26f32b771b7p-23 + z * 0x1.cf94c6515b7f6p-27)))))));
}

/*
 * erf below 1/2: x 2/sqrt(pi) + x^3 p0 exactly (x^2 and x^3 as pairs, the products exact, 2/sqrt(pi) and p0 as pairs, a
 * fast two-sum), the rest below 2^-7 of the result; out of line, so that the pieces' path saves no registers
 */
[[gnu::noinline]] static double erf_small(double x)
{
	struct __libc_dd square = __libc_fast_product(x, x);
	struct __libc_dd cube = __libc_fast_product(x, square.hi);
	struct __libc_dd term = __libc_fast_product(cube.hi, -0x1.812746b0379e7p-2);
	struct __libc_dd product = __libc_fast_product(x, 0x1.20dd750429b6dp+0);
	struct __libc_dd sum = __libc_dd_fast_sum(product.hi, term.hi);
	double z = square.hi;

	return sum.hi + (sum.lo + (product.lo + x * 0x1.1ae3a914fed80p-56 + term.lo + cube.hi * 0x1.ee12e49cab700p-57 + (cube.lo + x * square.lo) * -0x1.812746b0379e7p-2 + cube.hi * z * erf_series(z)));
}

/*
 * The ends (C23 7.12.8.1, F.10.5.1, and the strict file): +-0 and NaN return themselves, +-inf +-1; below 2^-60, x 2/sqrt(pi)
 * with 2/sqrt(pi) as a pair (underflow for a subnormal result; the x^3 term is below 2^-121 of it: dropped from 2^-28, as
 * the strict file does, it cost 0.03 ulp); from 6, +-(1 - tiny) (erfc 6 is below 2^-55)
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

	/* At run time, so that it is inexact and rounds in every direction */
	return __builtin_copysign(1.0 - tiny, x);
}

/*
 * The fast erf of the double-FPU cores (PLAN D162, D204), in double with one rounding:
 * - below 1/2, x 2/sqrt(pi) + x^3 p0 + x^5 Q(x^2), the first two terms formed exactly (x^2, x^3 and both products as
 *   pairs, 2/sqrt(pi) a pair too, a fast two-sum), relative however small x is: rounded with the rest, x^3 p0 (7 % of the
 *   result at 1/2) cost 0.76 ulp;
 * - from 1/2 to 6, 23 pieces of width 1/4 about m = round(4 |x|)/4, c0 + c1 t + t^2 Q(t) with t = |x| - m exact; c0 + c1 t
 *   formed exactly (two fused multiply-adds, or Dekker's product and a fast two-sum: |c1 t| <= 0.11 < c0), so only
 *   c1_lo + t Q (below 0.03 of the result) rounds in plain double before the one rounding;
 * - from 6, +-1 (inexact).
 * No exponential: the pieces approximate erf itself, which from 1/2 is at least 0.52, so an absolute 2^-59 is enough.
 */
double erf(double x)
{
	uint64_t bits = __libc_double_bits(x);
	uint32_t high = (uint32_t)(bits >> 32) & 0x7fffffffu;
	double a = __builtin_fabs(x);
	struct __libc_dd product;
	struct __libc_dd sum;
	const double *c;
	double lo;
	double kd;
	double q;
	double r;
	double t;

	/* 2^-60 <= |x| < 6, from the high word */
	if (high - 0x3c300000u >= 0x40180000u - 0x3c300000u)
		return erf_ends(x);

	if (high < 0x3fe00000u)
		return erf_small(x);

	/* m = round(4 |x|)/4 by the 1.5 2^52 shift, t = |x| - m exact (|t| <= 1/8, m >= 1/2); the entry for m from 1/2 */
	kd = a * 4.0 + 0x1.8p52;
	c = pieces[(uint32_t)__libc_double_bits(kd) - 2];
	kd -= 0x1.8p52;
	t = a - kd * 0.25;
	/* c1_lo + t Q(t) */
	q = c[3] + t * (c[4] + t * (c[5] + t * (c[6] + t * (c[7] + t * (c[8] + t * (c[9] + t * (c[10] + t * (c[11] + t * (c[12] + t * c[13])))))))));
	if (__libc_fpu_fma) {
		/* c0 + c1 t rounded, and its error by a second fused multiply-add (c0 - hi exact: they are within a factor of 2) */
		r = __builtin_fma(c[2], t, c[0]);
		lo = __builtin_fma(c[2], t, c[0] - r);
	} else {
		product = __libc_fast_product(c[2], t);
		sum = __libc_dd_fast_sum(c[0], product.hi);
		r = sum.hi;
		lo = sum.lo + product.lo;
	}
	r += lo + (c[1] + t * q);

	return (bits >> 63) != 0 ? -r : r;
}
