/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * near.inc
 *
 * Created on: Oct 21, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

/*
 * Arguments whose sin, cos, tan, erf, erfc or log10 lies so near the point
 * halfway between two floats that the Armv6-M fixed-point fast path
 * (PLAN D87, D88, D114) rounds it the wrong way without its error test (found over every binade on the host),
 * with the correctly rounded result (mpmath, 200 bits). Two tanf arguments
 * found (0x1.3240bcp-3, 0x1.b79bf6p+18) are left out: the float-float
 * quotient (D60) that every float-float board uses misses them as well, within the
 * allowance of D57; so does the float-float log10f (D58) for one log10f argument
 * found (0x1.c6c5cep+127).
 */
	{ sinf, 0x1.d3113p+24f, -0x1.6dcd46p-3f },
	{ cosf, 0x1.42bf9cp+10f, -0x1.f5b972p-1f },
	{ cosf, 0x1.50370cp+2f, 0x1.07a9f2p-1f },
	{ sinf, 0x1.4775ccp-1f, 0x1.319674p-1f },
	{ cosf, 0x1.c1fdd2p+8f, -0x1.78cdaep-1f },
	{ cosf, 0x1.1bc91ep+8f, 0x1.022df4p-1f },
	{ cosf, 0x1.dbfdb6p+11f, 0x1.e64d68p-1f },
	{ cosf, 0x1.8c1ea4p+24f, 0x1.306f46p-4f },
	{ sinf, 0x1.b8a7fap+26f, -0x1.cee35cp-1f },
	{ sinf, 0x1.ff5d1ap+25f, 0x1.fb1cd8p-1f },
	{ cosf, 0x1.412a82p+0f, 0x1.3e75e6p-2f },
	{ sinf, 0x1.a36f2cp+9f, -0x1.0311eep-4f },
	{ cosf, 0x1.9b1b72p+27f, 0x1.2c68d2p-2f },
	{ cosf, 0x1.465576p+21f, 0x1.ff7794p-2f },
	{ cosf, 0x1.c59074p-5f, 0x1.ff3728p-1f },
	{ sinf, 0x1.fdcc06p+2f, 0x1.fcd0bp-1f },
	{ sinf, 0x1.c7390cp+16f, 0x1.4e58e8p-2f },
	{ cosf, 0x1.3d9938p+7f, -0x1.2fac28p-3f },
	{ cosf, 0x1.bbfb94p+0f, -0x1.4d617ap-3f },
	{ sinf, 0x1.8a091p+13f, -0x1.e0d618p-1f },
	{ cosf, 0x1.f41d94p+26f, 0x1.919c8ep-2f },
	{ sinf, 0x1.3606f8p+0f, 0x1.df394cp-1f },
	{ cosf, 0x1.f07c04p+3f, -0x1.f682d4p-1f },
	{ sinf, 0x1.7eee04p+1f, 0x1.31f4c6p-3f },
	{ sinf, 0x1.f563bcp+28f, -0x1.81d15p-2f },
	{ sinf, 0x1.973bd6p-7f, 0x1.973928p-7f },
	{ sinf, 0x1.13079ap+30f, -0x1.fea94p-1f },
	{ cosf, 0x1.6ef82p+17f, -0x1.1c6a62p-1f },
	{ sinf, 0x1.6049d2p+8f, 0x1.aadc3ap-2f },
	{ cosf, 0x1.8968c2p+22f, -0x1.23c11cp-3f },
	{ sinf, 0x1.d12cep-12f, 0x1.d12cep-12f },
	{ sinf, 0x1.6c9676p+30f, 0x1.cda8fp-1f },
	{ tanf, 0x1.a52c6ep-9f, 0x1.a52ccep-9f },
	{ tanf, 0x1.24ad9p+7f, -0x1.eb2c26p+1f },
	{ tanf, 0x1.862ee2p+6f, 0x1.42f826p-3f },
	{ tanf, 0x1.0296dap+20f, -0x1.c2c9f2p-5f },
	{ tanf, 0x1.14562ap+7f, -0x1.facf16p-5f },
	{ tanf, 0x1.8b524cp-7f, 0x1.8b5736p-7f },
	{ tanf, 0x1.a5e26ap+29f, 0x1.0dd5d8p+1f },
	{ tanf, 0x1.cb6becp-7f, 0x1.cb73a2p-7f },
	{ tanf, 0x1.5445c6p+13f, -0x1.3b3356p-5f },
	{ tanf, 0x1.9cd92ep+5f, 0x1.110c86p+2f },
	{ tanf, 0x1.33d6bap+10f, -0x1.33e2e6p-3f },
	{ tanf, 0x1.be8826p-5f, 0x1.bef98p-5f },
	{ tanf, 0x1.178048p+7f, 0x1.3dac3ap+4f },
	{ tanf, 0x1.c6037p-9f, 0x1.c603e8p-9f },
	{ erff, 0x1.ab9f6p-6f, 0x1.e2692ep-6f },
	{ erff, 0x1.b7f46cp-4f, 0x1.ee8884p-4f },
	{ erff, 0x1.fab3bp-13f, 0x1.1de03ep-12f },
	{ erff, -0x1.392b0ep-6f, -0x1.615456p-6f },
	{ erff, 0x1.a156b8p-7f, 0x1.d6e412p-7f },
	{ erff, -0x1.2be05ap-18f, -0x1.525fdp-18f },
	{ erff, 0x1.baf22cp-15f, 0x1.f3cfap-15f },
	{ erff, -0x1.eb2d7ap-9f, -0x1.151db4p-8f },
	{ erff, 0x1.356cf8p-9f, 0x1.5d261ap-9f },
	{ erff, 0x1.a3fca4p-10f, 0x1.d9e77p-10f },
	{ erff, 0x1.aa6e38p-7f, 0x1.e125fp-7f },
	{ erff, 0x1.ab5558p-16f, 0x1.e231bp-16f },
	{ erff, 0x1.006efp-5f, 0x1.214274p-5f },
	{ erff, 0x1.1c7d24p-1f, 0x1.22d2aap-1f },
	{ erff, -0x1.8c8f9ep-4f, -0x1.be13b6p-4f },
	{ erff, -0x1.79cb5ep-19f, -0x1.aa4b9cp-19f },
	{ erfcf, 0x1.71bef4p+1f, 0x1.717f96p-15f },
	{ erfcf, 0x1.fadfbcp+2f, 0x1.9b739ap-95f },
	{ erfcf, 0x1.c9c304p+1f, 0x1.c7df34p-22f },
	{ erfcf, 0x1.b4ab74p+2f, 0x1.2ba978p-71f },
	{ erfcf, 0x1.f49388p+2f, 0x1.ea220cp-93f },
	{ erfcf, -0x1.50c45ap-18f, 0x1.00006p+0f },
	{ erfcf, -0x1.aeb4eep-17f, 0x1.0000f4p+0f },
	{ erfcf, 0x1.53866p+1f, 0x1.70ff14p-13f },
	{ erfcf, 0x1.e9067p+1f, 0x1.1972bep-24f },
	{ erfcf, 0x1.fbbaf8p+2f, 0x1.4c40d4p-95f },
	{ erfcf, 0x1.8a3a28p+1f, 0x1.bd342ep-17f },
	{ erfcf, -0x1.1c0936p-15f, 0x1.000282p+0f },
	{ erfcf, 0x1.5051ep+1f, 0x1.a8ba84p-13f },
	{ erfcf, 0x1.69958p+1f, 0x1.0f5fdcp-14f },
	{ erfcf, 0x1.89120cp+2f, 0x1.15b71ep-58f },
	{ erfcf, 0x1.496cdcp+1f, 0x1.1e3fdcp-12f },
	{ log10f, 0x1.04e6dp-129f, -0x1.3698dap+5f },
	{ log10f, 0x1.78c6a2p-23f, -0x1.b05fccp+2f },
	{ log10f, 0x1.14db8cp-4f, -0x1.2b8be8p+0f },
	{ log10f, 0x1.2c46cep-1f, -0x1.da9ef6p-3f },
	{ log10f, 0x1.c80cc4p-1f, -0x1.9bb5e4p-5f },
	{ log10f, 0x1.ed2c76p-1f, -0x1.0a91a4p-6f },
	{ log10f, 0x1.fbe1a6p-1f, -0x1.cbc02ep-9f },
	{ log10f, 0x1.01715ep+0f, 0x1.3fed74p-9f },
	{ log10f, 0x1.13ae68p+0f, 0x1.0783bap-5f },
	{ log10f, 0x1.c50308p+0f, 0x1.fba30ep-3f },
	{ log10f, 0x1.b650b4p+4f, 0x1.700aecp+0f },
	{ log10f, 0x1.b7ed5ap+19f, 0x1.7d19fcp+2f },
