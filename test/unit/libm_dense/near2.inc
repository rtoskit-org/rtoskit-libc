/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * near2.inc
 *
 * Created on: Oct 26, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

/*
 * Arguments whose powf lies so near the point halfway between two floats
 * that the fixed-point fast path (PLAN D115) rounds it the wrong way without
 * its error test (found on the host among a billion structured pairs), with
 * the correctly rounded result (mpmath, 400 bits).
 */
	{ 0x1.ad2f4cp-60f, 0x1.0b0cccp+1f, 0x1.4c1ff8p-124f },
	{ 0x1.c76888p-126f, 0x1.0e223ep-1f, 0x1.f2133p-67f },
	{ 0x1.5fa726p+66f, -0x1.7b5cb2p-2f, 0x1.4cf85cp-25f },
	{ 0x1.16929p+0f, 0x1.d098ecp+9f, 0x1.363f6cp+113f },
	{ 0x1.d5c358p-1f, -0x1.32d46p+8f, 0x1.149048p+38f },
	{ 0x1.2fdda2p+0f, 0x1.ed8b6cp+8f, 0x1.08cep+122f },
	{ 0x1.b189ep-40f, 0x1p+1f, 0x1.6f19dap-79f },
	{ 0x1.63a5eap-4f, 0x1.6p+4f, 0x1.5a0742p-78f },
	{ 0x1.446ed8p+5f, -0x1.8p+1f, 0x1.f716ccp-17f },
	{ 0x1.c92e8ap+34f, -0x1.4bf754p-17f, 0x1.ffe0b2p-1f },
	{ 0x1.201d8p-13f, 0x1.c74c2ap-12f, 0x1.fe06ep-1f },
	{ 0x1.ba7698p-36f, 0x1.21dc4p-18f, 0x1.fff23p-1f },
