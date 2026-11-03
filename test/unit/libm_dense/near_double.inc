/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * near_double.inc
 *
 * Created on: Nov 3, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

/*
 * Arguments whose sin, cos or tan (P9.03) lies so near the point halfway between two
 * doubles that the fixed-point kernel of the speed flavour (P8.03, PLAN D133)
 * rounds its 64-bit result the wrong way without its error test, with the
 * correctly rounded result (MPFR, 256 bits), which the double-double path
 * also returns (tools/scripts/trig-fixed-check --near).
 */
	{ sin, -0x1.322397d34c20dp+3, 0x1.21f9ab7d1cc9p-3 },
	{ sin, 0x1.b38e54af7c9d4p+2, 0x1.feea809b9e43p-2 },
	{ cos, 0x1.077d6d91f7298p+0, 0x1.07e9bc53b6c96p-1 },
	{ cos, 0x1.daab6be76427fp+7, 0x1.27088d35f262bp-3 },
	{ cos, -0x1.f8c18b289694ep+2, -0x1.0ce7eb44c4233p-5 },
	{ cos, 0x1.facf104c78308p+2, -0x1.09aba6c6b4a89p-4 },
	{ sin, -0x1.b05d09afa4b24p+1, 0x1.df5851e232d2ap-3 },
	{ cos, -0x1.3b9ecdd24c38p+2, 0x1.bd4ae31fb54e7p-3 },
	{ sin, 0x1.2fa897ed92aefp-14, 0x1.2fa897e91f5d4p-14 },
	{ cos, 0x1.de65bd360559dp-23, 0x1.fffffffffff2p-1 },
	{ cos, 0x1.89a96a8c12388p+0, 0x1.0ebcb78289accp-5 },
	{ sin, 0x1.79519c2b34f3p+1, 0x1.8a66d4967196ep-3 },
	{ sin, 0x1.20cf1a3e91416p+3, 0x1.8e4a317702e09p-2 },
	{ sin, 0x1.cc1443863c50dp-7, 0x1.cc1064deb149p-7 },
	{ sin, -0x1.c1e1d604b0634p+1, 0x1.75436543952e2p-2 },
	{ sin, -0x1.6252830de0a68p+2, 0x1.5bd5f210d206fp-1 },
	{ cos, 0x1.38e46d50b42c6p-5, 0x1.ffa067941e41ap-1 },
	{ sin, 0x1.056f072b39f7bp+10, 0x1.9d5f9c1722744p-2 },
	{ sin, 0x1.fd781d72535dcp-7, 0x1.fd72dc46a8207p-7 },
	{ sin, 0x1.1c14265e51dep-1, 0x1.0db9d328c5517p-1 },
	{ sin, -0x1.a51a4d77529c4p+2, -0x1.2b3b26ee5d746p-2 },
	{ cos, 0x1.d6d89fa524f5ap-23, 0x1.fffffffffff27p-1 },
	{ sin, -0x1.c3ff0b087a92ap+2, -0x1.67ceea32e744ep-1 },
	{ sin, 0x1.0d60a1ea41f44p+11, -0x1.cd07d93bbebadp-4 },
	{ cos, 0x1.d2db9a7a1d258p+2, 0x1.0fad01b5d43ap-1 },
	{ sin, 0x1.211587f5da50cp-11, 0x1.21158700192b1p-11 },
	{ cos, -0x1.031079c431c79p+3, -0x1.ea5ab19fe0f37p-3 },
	{ cos, -0x1.4b6299df123d8p+2, 0x1.cba5759ec63a9p-2 },
	{ cos, -0x1.ef5477fbd5f8ap+2, 0x1.d3c4d13556d1ep-4 },
	{ cos, 0x1.ab5d457a06696p+9, 0x1.f428c8f4948a8p-1 },
	{ cos, 0x1.486be428b3374p+2, 0x1.a0cb84b52c69ap-2 },
	{ cos, -0x1.2979d2912ebb6p+2, -0x1.074ed54e92b92p-4 },
	{ tan, 0x1.029818804b1cep-11, 0x1.029819e01bd8cp-11 },
	{ tan, 0x1.6de54fc62ab37p+2, -0x1.45570cb4450efp-1 },
	{ tan, 0x1.0ae052d41a1dep-17, 0x1.0ae052d432495p-17 },
	{ tan, 0x1.2617623ceb75p+3, -0x1.e91770cda2fd2p-3 },
	{ tan, 0x1.77891ed726a6p+1, -0x1.afa3428fd7351p-3 },
	{ tan, 0x1.1c1ed00dcacccp+3, -0x1.371c4ebdc7873p-1 },
	{ tan, 0x1.e8cc3c21c2f4cp+1, 0x1.9b9e4faba6d94p-1 },
	{ tan, 0x1.77dfef89cad8p+1, -0x1.a9f9523e2f248p-3 },
	{ tan, 0x1.43db28ef86dap-11, 0x1.43db2ba295f0dp-11 },
	{ tan, 0x1.90a197963fea9p-5, 0x1.90f36fbfb55dp-5 },
	{ tan, -0x1.5af1a5cf16ddcp+1, 0x1.d6fbc2aec9be1p-2 },
	{ tan, 0x1.63297d576e4bep+1, -0x1.8982e8bb2dbc4p-2 },
	{ tan, 0x1.1b67eb0724a16p+3, -0x1.46fadfec13a3p-1 },
	{ tan, 0x1.6133c93d93e1ep+2, -0x1.eaefc5b5312c8p-1 },
	{ tan, 0x1.de9696d918da1p+0, -0x1.9fb809afef5d2p+1 },
	{ tan, 0x1.03a6f4ca5e971p-9, 0x1.03a70b0ccd204p-9 },
