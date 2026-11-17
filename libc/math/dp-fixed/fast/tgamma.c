/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * tgamma.c
 *
 * Created on: Nov 17, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include <math.h>
#include <stdint.h>

#include "dp-fixed-fast.h"
#include "internal/math-fast.h"

/*
 * Gamma(x) 2^-s on [1, 8) in 32 pieces (eight of width 1/8 on [1, 2), eight of 1/4 on [2, 4), sixteen of 1/4 on [4, 8))
 * at x = start + u width, u in [0, 1) in Q64 from x's fraction bits below the index's: b0 in Q64, then b1..b11 signed
 * in Q63, fitted here by least squares on Chebyshev nodes with the evaluation's truncations (within 2^-58.06 of Gamma
 * relatively), s per piece so that the values are below 1
 */
static const uint64_t pieces[32][12] = {
	{ 0x7ffffffffffffff8ull, 0xfb61dcc0e4127e0dull, 0x00fd32c611d7de03ull, 0xffe2f5ee70dcd269ull, 0x0003ed4a2016f3b3ull, 0xffff824dff9de1c1ull, 0x00000fe3e4378812ull, 0xfffffe0229638b9full, 0x0000003fb12be709ull, 0xfffffff83630d990ull, 0x00000000daa78c67ull, 0xfffffffff05edd4full },
	{ 0xf1160cb119e907afull, 0xfa256fd9470b98d3ull, 0x01730bc7f21fde03ull, 0xffdd8bf2bf6fa27full, 0x00045f54f86240baull, 0xffff854e6f0afe0aull, 0x00000de6bdbce532ull, 0xfffffe73927335b0ull, 0x0000002c1edba8f7ull, 0xfffffffb2ce3ecdeull, 0x000000007b043816ull, 0xfffffffff7cae434ull },
	{ 0xe809fe23f77751e6ull, 0xfcb38ce8e8b0a97aull, 0x0121d4e80717f85full, 0xffeb2670aa874505ull, 0x00029e3a24d56fb8ull, 0xffffbfe3073eb1d4ull, 0x000006a3f6aa296full, 0xffffff55fdeb52c8ull, 0x00000011140b7ea4ull, 0xfffffffe4fe264b0ull, 0x000000002765214full, 0xfffffffffd891e5aull },
	{ 0xe38fd6f47f22a123ull, 0xfec2060f459af84eull, 0x00f0d3c646bc0349ull, 0xfff38f07fc5ec8dfull, 0x0001adfa6487922full, 0xffffdcd2acfc45f7ull, 0x0000036a2eeccf7cull, 0xffffffb128804aa7ull, 0x000000073c30506aull, 0xffffffff59233e74ull, 0x000000000e057053ull, 0xffffffffff2ca41cull },
	{ 0xe2dfc48da77b553dull, 0x00847544772b0d9aull, 0x00d4626dd3106807ull, 0xfff92214f8d77752ull, 0x0001283bd37303c1ull, 0xffffec27730c9a25ull, 0x000001e0228f8019ull, 0xffffffd91664f086ull, 0x000000034e0b2ccbull, 0xffffffffba1b19deull, 0x000000000572ae8full, 0xffffffffffb26ff2ull },
	{ 0xe585e45b47201685ull, 0x021ce85c2f6fd9daull, 0x00c60ce2a6fe6d9dull, 0xfffd1d64073ef4efull, 0x0000dc9a2d64f547ull, 0xfffff4d29571abd4ull, 0x00000119459db5f2ull, 0xffffffebdcca6b89ull, 0x000000019c7363adull, 0xffffffffe0acafbaull, 0x000000000247a6e6ull, 0xffffffffffe14381ull },
	{ 0xeb47ae87cb8a101eull, 0x03a39ae9be58af1aull, 0x00c22fe2e947b2c2ull, 0x0000339022f0b8e4ull, 0x0000b2d4703bb9fcull, 0xfffffa102910683bull, 0x000000b0cbc558b6ull, 0xfffffff542d1fa28ull, 0x00000000d56845c2ull, 0xfffffffff1275001ull, 0x00000000010467cfull, 0xfffffffffff2ff27ull },
	{ 0x7a0a832c1fc0949dull, 0x0295a37128cd0f4eull, 0x006364dfb2ebfaf4ull, 0x00016802e7043f47ull, 0x00004f1e90c4e87bull, 0xfffffebdd9f9b419ull, 0x0000003c6afcbec3ull, 0xfffffffd207d7265ull, 0x000000003a9c4930ull, 0xfffffffffc52be68ull, 0x00000000003d7f49ull, 0xfffffffffffd178bull },
	{ 0x7ffffffffffffffcull, 0x06c3b981c824f5aaull, 0x01a5b978b96b8534ull, 0x0014e239984ddcd1ull, 0x0004c07eebf3229eull, 0xfffffee80e32ae99ull, 0x00000b6bf14ac8a2ull, 0xffffff451b2fbb69ull, 0x0000002260ae949dull, 0xfffffffc55cdeeb2ull, 0x000000006dfb374dull, 0xfffffffff85ee20bull },
	{ 0x91063ed67aaa932full, 0x0a611005625448cdull, 0x0201777e4be83fa2ull, 0x0028aaa75828a33bull, 0x000554b61b83cecbull, 0x00003a30d2c68e66ull, 0x0000092ae143beb3ull, 0xfffffffdf91f951bull, 0x0000000fdc143300ull, 0xffffffff16c88cafull, 0x00000000215f6021ull, 0xfffffffffe29f9e5ull },
	{ 0xaa27d36a3d9c7fedull, 0x0ef4ac2f8d3849d0ull, 0x029e449a96fbfbabull, 0x0040fdb4be9466c4ull, 0x000704e2e24cbac2ull, 0x0000742a6f322d93ull, 0x00000aa44def5b79ull, 0x0000006958cb4e6full, 0x0000000c5abd28e3ull, 0x000000000604bfdfull, 0x000000000ec4bf2full, 0xffffffffffb540aaull },
	{ 0x66ef5c5b690c670dull, 0x0a896500c4e9e9c2ull, 0x01c8472a50088d39ull, 0x00314071349c8313ull, 0x0004fd97231ad2d8ull, 0x00005fb6af146cd7ull, 0x000007769c33d514ull, 0x00000069ca65d221ull, 0x000000077cf19c80ull, 0x000000004615e90cull, 0x0000000005f13a21ull, 0x0000000000292bdaull },
	{ 0x8000000000000000ull, 0x0ec3b981c824f392ull, 0x027e30a8f2708806ull, 0x00499968af73fa1cull, 0x00075cc61f43b9b7ull, 0x000096f7ea1ce439ull, 0x00000b48f8c611e0ull, 0x000000b28bc60f98ull, 0x0000000b199cf4daull, 0x000000008b557421ull, 0x0000000007b00b92ull, 0x0000000000674922ull },
	{ 0xa32706b149ffe596ull, 0x14bd95f376497a99ull, 0x038dc86ec1afe8c7ull, 0x006deeec0ca8fc4eull, 0x000b14a1ca0a6a24ull, 0x0000ec0db02e9693ull, 0x0000119659249a4full, 0x000001231127bf34ull, 0x000000119be9392aull, 0x00000000efb68f44ull, 0x000000000bff4897ull, 0x0000000000bb123cull },
	{ 0x6a58e4226681cff4ull, 0x0eaa2a390a301217ull, 0x029235a3973103c8ull, 0x005282daa08c5cbdull, 0x000872e9195a90efull, 0x0000b8e8b39c8505ull, 0x00000de957c46887ull, 0x000000ec1c1c0ea9ull, 0x0000000e4eabc80dull, 0x00000000c900c4b4ull, 0x0000000009e47384ull, 0x0000000000a19edaull },
	{ 0x8d891efdb0710db2ull, 0x14ebe0a6c55267e7ull, 0x03c48e7a46a8fd32ull, 0x007cc180f2586edfull, 0x000d04bdf6d6e846ull, 0x0001234e15282b7cull, 0x00001639ec8a689dull, 0x000001804a1ce387ull, 0x0000001784a35bceull, 0x000000015077149dull, 0x00000000109d261full, 0x000000000115aebfull },
	{ 0x6000000000000000ull, 0x0f12cb21561bb6bcull, 0x02cae016d256b282ull, 0x005f161912be36faull, 0x000a1f2b226820d9ull, 0x0000e7065194c761ull, 0x000011e6390f2cdcull, 0x0000013a78bdd7c8ull, 0x000000137b5f19b2ull, 0x000000011ab38115ull, 0x000000000e1394bcull, 0x0000000000ef38acull },
	{ 0x848fb5700c1fea8aull, 0x15f3420b5a6bb2e3ull, 0x042f0c3934c37fdbull, 0x00922ea6b664a79bull, 0x000fdfb234ef8f50ull, 0x000171153bda3030ull, 0x00001d0b0329bb5full, 0x00000205e42ac8cfull, 0x000000207eb5d907ull, 0x00000001dd990891ull, 0x0000000018027e2full, 0x00000000019e22e2ull },
	{ 0x5d0dc79e19b195f6ull, 0x1027ac12fc1e1e6bull, 0x032a9192b4ede02cull, 0x007155d985ee30e7ull, 0x000c8cb9a03506adull, 0x000128fa2ecf9e81ull, 0x000017bab7c86010ull, 0x000001ad2ea621cfull, 0x0000001b45b22743ull, 0x0000000195bbafabull, 0x000000001498b0e5ull, 0x0000000001685514ull },
	{ 0x84b08d0dd569fcd6ull, 0x18096b944680c9feull, 0x04d7039d0e938c8aull, 0x00b13e50879e0064ull, 0x0014008a2689dcb4ull, 0x0001e16513604e5dull, 0x0000270b2eab0246ull, 0x000002cbe53d633cull, 0x0000002e0f6ff17full, 0x00000002b556ac48ull, 0x0000000023892808ull, 0x0000000002765447ull },
	{ 0x6000000000000000ull, 0x1212cb21561bb6deull, 0x03bc0cc8e7b8679eull, 0x008bc41a7fe417d7ull, 0x0010108cb38f9687ull, 0x000188f903d474d4ull, 0x000020569dcdca72ull, 0x00000258dd25def9ull, 0x000000272196a936ull, 0x0000000253cd190eull, 0x000000001ed31520ull, 0x00000000022a321cull },
	{ 0x8cd8b0c70ce1e932ull, 0x1b76f3d790736da1ull, 0x05d1311d7db66818ull, 0x00de4254b517f5dcull, 0x001a0097c39d27e7ull, 0x00028621b3125f54ull, 0x000035ed067cfb81ull, 0x000003f6f4146b4dull, 0x00000042e296bed4ull, 0x0000000405c5aaf9ull, 0x0000000035c423a0ull, 0x0000000003d38d0full },
	{ 0x68af8091dce7c8b4ull, 0x15150fd24c6f6f1bull, 0x04925e863b4d7495ull, 0x00b229ade1fb8659ull, 0x001533ae6c93dea5ull, 0x000216e50ed2cd12ull, 0x00002d41b125cba8ull, 0x0000035e813b6872ull, 0x000000397f52318eull, 0x000000037ee600a8ull, 0x000000002f247253ull, 0x0000000003661b62ull },
	{ 0x9d91a7806d6ddc3eull, 0x20b0b4288264401dull, 0x073feb03c5b732dfull, 0x011fea3971f60f75ull, 0x0022d4891631ea49ull, 0x00037bb0a9ad4ee0ull, 0x00004c7397b21fe2ull, 0x000005c2d55a599full, 0x000000636d2809cdull, 0x000000061bf71238ull, 0x00000000531b2d0eull, 0x00000000061193b1ull },
	{ 0x77ffffffffffffffull, 0x19977de9aba2a4e5ull, 0x05cc3cad37082e51ull, 0x00ea75edae59b509ull, 0x001cd0f188677a92ull, 0x0002ec40103cdd33ull, 0x000040fbd4ac67cdull, 0x000004f4803e85cfull, 0x0000005674ba418eull, 0x000000055e11c249ull, 0x0000000049a292a3ull, 0x000000000571dcefull },
	{ 0x5c6e3402a0744108ull, 0x143972c892ff47dcull, 0x04acffda170342ceull, 0x00c0651082c65dabull, 0x001802763dfed00bull, 0x0002780adbd482aeull, 0x000037949920e645ull, 0x000004497a19b6dbull, 0x0000004b999ac317ull, 0x00000004bdb6a1d7ull, 0x00000000419444e2ull, 0x0000000004e8b77aull },
	{ 0x8ff150c88fbeb3f7ull, 0x204271c5b8007787ull, 0x079a92f5b6516045ull, 0x013e1f377a705c50ull, 0x002849aab35aed7full, 0x000432b5db86aa1full, 0x00005fa8a3373d22ull, 0x000007760fd2793bull, 0x00000084f24ecdf0ull, 0x000000086b7f1741ull, 0x0000000075671422ull, 0x0000000008e520e9ull },
	{ 0x7140b0644ea6f64cull, 0x19f5481b1f6dc5f4ull, 0x063b768bfa2eba0eull, 0x0108efb168181315ull, 0x0022081453752e10ull, 0x0003978b42f7e0f3ull, 0x000052d0992ee8f8ull, 0x00000687a8d108ddull, 0x0000007588ce6bbcull, 0x000000078405cd3bull, 0x00000000699dcb03ull, 0x000000000818ecefull },
	{ 0x59ffffffffffffffull, 0x15119e6f40b9fc0full, 0x0525e97136a3257cull, 0x00de3a57ac7ce051ull, 0x001cf06493b38136ull, 0x000317b798ba3bdcull, 0x0000481edefaf192ull, 0x000005bf4147037aull, 0x0000006877ae1026ull, 0x00000006be3f6318ull, 0x000000005f7bb5cbull, 0x000000000767f5cdull },
	{ 0x906c31441ab5a59cull, 0x227d34f97ab28302ull, 0x0891e6f14d24ebafull, 0x01776de76dc8913dull, 0x00318a29c8f34423ull, 0x00055bb85be03f8cull, 0x00007e58db242665ull, 0x00000a2c1cc46636ull, 0x000000bab0a75a12ull, 0x0000000c2953a327ull, 0x00000000ad7cac31ull, 0x000000000d9c65c8ull },
	{ 0x74f411a2f4caf237ull, 0x1c75c1b3c7bf5c8full, 0x072faaf5d1e203e1ull, 0x013f4df4c1300695ull, 0x002aacd46d784d07ull, 0x0004ab811870199aull, 0x00006f4eb1c95749ull, 0x0000090d35f4ac0full, 0x000000a7af0a9cbfull, 0x0000000b0586ab52ull, 0x000000009e5dbb7eull, 0x000000000c909228ull },
	{ 0x5f8e94d4a25cdfcfull, 0x17abf79873bf3b73ull, 0x0611d646f412bfcaull, 0x011165f20fa7c440ull, 0x0024fe4eb1986c8cull, 0x0004181e2389e90eull, 0x0000629c59ce4856ull, 0x00000818feddf9edull, 0x0000009762f1f3c5ull, 0x0000000a09b07efbull, 0x00000000913fef5dull, 0x000000000ba6d6a2ull },
};

static const uint8_t shifts[32] = { 1, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7, 7, 8, 9, 9, 10, 11, 11, 12, 13 };

/* c + u p in c's units for u in Q64 (unsigned) and a signed p, the product truncated toward zero */
static inline int64_t step(int64_t c, uint64_t u, int64_t p)
{
	return c + (p < 0 ? -(int64_t)__libc_trig_high64(u, -(uint64_t)p) : (int64_t)__libc_trig_high64(u, (uint64_t)p));
}

/* Gamma 2^-s in Q64 at u for the piece j */
static inline uint64_t piece(int j, uint64_t u)
{
	const uint64_t *c = pieces[j];
	int64_t q = step((int64_t)c[10], u, (int64_t)c[11]);

	for (int k = 9; k > 0; --k)
		q = step((int64_t)c[k], u, q);

	return c[0] + (q < 0 ? -(__libc_trig_high64(u, -(uint64_t)q) << 1) : __libc_trig_high64(u, (uint64_t)q) << 1);
}

/*
 * S(z) = (lgamma(x) - (x - 1/2) log x + x - log(2 pi)/2) x for z = 1/x^2 <= 1/64, in w = 64 z: b0 in Q64, b1..b6 signed in
 * Q63 (within 2^-60.38 absolutely; divided by x from 8, below 2^-63)
 */
static const uint64_t stirling[7] = { 0x155555555555554aull, 0xfffe93e93e93eb59ull, 0x000001a01a017e2dull, 0xfffffffb1fb2c934ull, 0x000000001b92c63cull, 0xffffffffff0836a0ull, 0x00000000000a60e0ull };

/* b0 + u Q(u) in Q64 for a row of n + 1 coefficients (signed Horner's rule from b_n) */
static inline uint64_t stirling_series(const uint64_t *c, int n, uint64_t u)
{
	int64_t q = step((int64_t)c[n - 1], u, (int64_t)c[n]);

	for (int k = n - 2; k > 0; --k)
		q = step((int64_t)c[k], u, q);

	return c[0] + (q < 0 ? -(__libc_trig_high64(u, -(uint64_t)q) << 1) : __libc_trig_high64(u, (uint64_t)q) << 1);
}

/* a b as a 128-bit (hi, lo), from four 32 x 32 products */
static inline void tgamma_product(uint64_t a, uint64_t b, uint64_t *hi, uint64_t *lo)
{
	uint64_t p00 = __libc_trig_wide((uint32_t)a, (uint32_t)b);
	uint64_t p01 = __libc_trig_wide((uint32_t)a, (uint32_t)(b >> 32));
	uint64_t p10 = __libc_trig_wide((uint32_t)(a >> 32), (uint32_t)b);
	uint64_t middle = (p00 >> 32) + (uint32_t)p01 + (uint32_t)p10;

	*lo = (middle << 32) | (uint32_t)p00;
	*hi = __libc_trig_wide((uint32_t)(a >> 32), (uint32_t)(b >> 32)) + (p01 >> 32) + (p10 >> 32) + (middle >> 32);
}

/* ln x in Q64 as (hi, lo), signed, for x = m 2^(e - 52): ln z by the fixed log kernel, z = m 2^-52 in [45/64, 90/64) (halved from 90/64, e one higher), plus e ln 2 */
static inline void tgamma_log(uint64_t m, int32_t e, uint64_t *hi, uint64_t *lo)
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
 * Gamma(x) for 8 <= x < 64 as e^E, E = (x - 1/2) ln x - x + log(2 pi)/2 + S(1/x^2)/x in 128-bit Q64: ln x by the fixed
 * log kernel plus e ln 2 (a few units of 2^-64), times x - 1/2 exactly but for units of 2^-127 (64 x 128 bits, the top
 * 128 kept), x exactly, S by lgamma's fit (its series in 64/x^2, 1/x by __libc_dp_fixed_reciprocal); E is within about
 * 2^-56 absolutely below 64. Then t = E log2(e) = n + f 2^-64 (E times log2(e) in Q127, the product's top bits) and
 * 2^(f 2^-64) by exp's kernel, rounded once.
 */
static double tgamma_stirling_fixed(uint64_t m, int32_t e)
{
	constexpr uint64_t c_hi = 0xb8aa3b295c17f0bbull;
	constexpr uint64_t c_lo = 0xbe87fed0691d3e88ull;
	uint64_t xm = (m << 11) - (1ull << (62 - e));
	uint64_t y = __libc_dp_fixed_reciprocal(m << 11);
	uint64_t l_hi;
	uint64_t l_lo;
	uint64_t a_hi;
	uint64_t a_lo;
	uint64_t b_hi;
	uint64_t b_lo;
	uint64_t e_hi;
	uint64_t e_lo;
	uint64_t x_lo;
	uint64_t add;
	uint64_t t_hi;
	uint64_t t_lo;
	int32_t n;

	tgamma_log(m, e, &l_hi, &l_lo);
	/* (x - 1/2) ln x = (xm 2^(e - 63))(L 2^-64): xm L's top 128 bits P, then P 2^(e - 63) in Q64 is P << (e + 1) */
	tgamma_product(xm, l_lo, &a_hi, &a_lo);
	__libc_dp_fixed_k_product((uint32_t)l_hi, xm, &b_hi, &b_lo);
	b_lo += a_hi;
	b_hi += b_lo < a_hi;
	e_hi = (b_hi << (e + 1)) | (b_lo >> (63 - e));
	e_lo = b_lo << (e + 1);
	/* - x, x 2^64 = m << (12 + e) */
	x_lo = m << (12 + e);
	e_hi -= (m >> (52 - e)) + (e_lo < x_lo);
	e_lo -= x_lo;
	/* + log(2 pi)/2 + S/x in Q64 (the series in 64/x^2 = high(y, y) >> (2 e - 6), 1/x = y 2^(-64 - e)) */
	add = 0xeb3f8e4325f5a535ull + (__libc_trig_high64(stirling_series(stirling, 6, __libc_trig_high64(y, y) >> (2 * e - 6)), y) >> e);
	e_lo += add;
	e_hi += e_lo < add;
	/* t = E log2(e): (E_hi 2^64 + E_lo)(c_hi 2^64 + c_lo) 2^-127, E_hi below 2^8 */
	__libc_dp_fixed_k_product((uint32_t)e_hi, c_hi, &t_hi, &t_lo);
	add = __libc_trig_high64(e_lo, c_hi);
	t_lo += add;
	t_hi += t_lo < add;
	add = (uint64_t)(uint32_t)e_hi * (c_lo >> 32);
	add >>= 32;
	t_lo += add;
	t_hi += t_lo < add;
	n = (int32_t)((t_hi << 1) | (t_lo >> 63));

	return __libc_dp_fixed_double(0, n, __libc_dp_fixed_exp2_fraction(t_lo << 1));
}

/* The piece and u of y = m 2^(e - 52) in [1, 8), m the 53-bit significand, 0 <= e <= 2 */
static inline int tgamma_index(uint64_t m, int32_t e, uint64_t *u)
{
	if (e < 2) {
		*u = m << 15;
		return 8 * e + (int)((m >> 49) & 7);
	}
	*u = m << 16;

	return 16 + (int)((m >> 48) & 15);
}

/*
 * The fast tgamma of the soft-double cores (PLAN D162, D219, D231), Gamma on [2^-54, 8) in 64-bit fixed point with no
 * soft-double operation on that path, within 1 ulp (docs/research/libm-fast-double.md, section 35):
 * - the integers to 23 exactly ((n - 1)!, from the double file's table);
 * - on [1, 8), the pieces, Gamma(x) = V 2^s, one rounding;
 * - below 1, Gamma(1 + x)/x: 1 + x in Q63 (its truncation only picks the piece and moves u), the piece's value times 1/x
 *   by __libc_dp_fixed_reciprocal (2^-63 relatively);
 * - from 8 to 64, Stirling's exponential in fixed point (tgamma_stirling_fixed); from 64, negative x and the ends by
 *   the double file's paths (__libc_fast_tgamma_large, _negative, _ends: there the fixed log's few units of 2^-64,
 *   times x, would no longer stay well within 1 ulp).
 * These cores round to nearest and raise no flags (D44). C23 7.12.8.4.
 */
double tgamma(double x)
{
	uint64_t bits = __libc_double_bits(x);
	uint64_t magnitude = bits & 0x7fffffffffffffffull;
	int32_t e = (int32_t)(magnitude >> 52) - 1023;
	uint64_t m = (magnitude & 0x000fffffffffffffull) | 0x0010000000000000ull;
	uint64_t u;
	uint64_t v;
	int j;

	if (magnitude - 0x3c90000000000000ull >= 0x4330000000000000ull - 0x3c90000000000000ull || x > 171.62437695630272)
		return __libc_fast_tgamma_ends(x);
	if ((bits >> 63) != 0)
		return __libc_fast_tgamma_negative(x);
	/* An integer to 23: no fraction bits below the point */
	if (e >= 0 && e < 5 && (m & ((1ull << (52 - e)) - 1)) == 0 && (m >> (52 - e)) <= 23)
		return __libc_fast_tgamma_factorials[(m >> (52 - e)) - 1];
	if (e >= 6)
		return __libc_fast_tgamma_large(x);
	if (e >= 3)
		return tgamma_stirling_fixed(m, e);
	if (e >= 0) {
		j = tgamma_index(m, e, &u);
		return __libc_dp_fixed_normal(0, shifts[j] - 1, piece(j, u));
	} else {
		/* y = 1 + x in Q63 (y in [1, 2): the piece's exponent 0), V(y) 2^s / x = high(V, y') 2^(s - 64 - e), y' = 2^127/(m 2^11) */
		uint64_t y = 0x8000000000000000ull + (e >= -11 ? m << (11 + e) : m >> (-11 - e));

		j = (int)((y >> 60) & 7);
		v = piece(j, y << 4);

		return __libc_dp_fixed_normal(0, shifts[j] - 1 - e, __libc_trig_high64(v, __libc_dp_fixed_reciprocal(m << 11)));
	}
}
