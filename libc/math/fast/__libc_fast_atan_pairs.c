/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * __libc_fast_atan_pairs.c
 *
 * Created on: Nov 13, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include "internal/math-fast.h"

/* atan(j/16) for j = 0..16, each as the double nearest it and the double nearest the rest (PLAN D198) */
const struct __libc_dd __libc_fast_atan_pairs[17] = {
	{ 0x0.0p+0, 0x0.0p+0 },
	{ 0x1.ff55bb72cfdeap-5, -0x1.c934d86d23f1dp-60 },
	{ 0x1.fd5ba9aac2f6ep-4, -0x1.cd37686760c17p-59 },
	{ 0x1.7b97b4bce5b02p-3, 0x1.347b0b4f881cap-58 },
	{ 0x1.f5b75f92c80ddp-3, 0x1.8ab6e3cf7afbdp-57 },
	{ 0x1.362773707ebccp-2, -0x1.963a544b672d8p-57 },
	{ 0x1.6f61941e4def1p-2, -0x1.c63aae6f6e918p-56 },
	{ 0x1.a64eec3cc23fdp-2, -0x1.24dec1b50b7ffp-56 },
	{ 0x1.dac670561bb4fp-2, 0x1.a2b7f222f65e2p-56 },
	{ 0x1.0657e94db30d0p-1, -0x1.d5b495f6349e6p-56 },
	{ 0x1.1e00babdefeb4p-1, -0x1.928df287a668fp-58 },
	{ 0x1.345f01cce37bbp-1, 0x1.1021137c71102p-55 },
	{ 0x1.4978fa3269ee1p-1, 0x1.2419a87f2a458p-56 },
	{ 0x1.5d58987169b18p-1, 0x1.0028e4bc5e7cap-57 },
	{ 0x1.700a7c5784634p-1, -0x1.8c34d25aadef6p-56 },
	{ 0x1.819d0b7158a4dp-1, -0x1.bf76229d3b917p-56 },
	{ 0x1.921fb54442d18p-1, 0x1.1a62633145c07p-55 },
};
