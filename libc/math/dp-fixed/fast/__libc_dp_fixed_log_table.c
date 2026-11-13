/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * __libc_dp_fixed_log_table.c
 *
 * Created on: Nov 13, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include <stdint.h>

#include "dp-fixed-fast.h"

/*
 * For z in [45/64 + j/128, 45/64 + (j + 1)/128): invc = 1/c in Q31, c the middle, rounded, and -log(invc) in Q64 for
 * that invc exactly, rounded (PLAN D192; generated with mpmath at 300 bits); |z invc - 1| <= 2^-7.5
 */
const struct __libc_dp_fixed_log_entry __libc_dp_fixed_log_table[90] = {
	{ .logc = -0x58c00c2d78b124eell, .invc = 0xb509e68bu },
	{ .logc = -0x55efdd4fcd47eb7bll, .invc = 0xb30f6353u },
	{ .logc = -0x532782784cb0efbbll, .invc = 0xb11fd3b8u },
	{ .logc = -0x5066d09011a31c87ll, .invc = 0xaf3addc7u },
	{ .logc = -0x4dad9ddae8445bb3ll, .invc = 0xad602b58u },
	{ .logc = -0x4afbc1f42c4d4e7dll, .invc = 0xab8f69e3u },
	{ .logc = -0x485115b4cae350fcll, .invc = 0xa9c84a48u },
	{ .logc = -0x45ad732ea3edcd67ll, .invc = 0xa80a80a8u },
	{ .logc = -0x4310b59d3f8b8c46ll, .invc = 0xa655c439u },
	{ .logc = -0x407ab9593f1a43ddll, .invc = 0xa4a9cf1eu },
	{ .logc = -0x3deb5bca39ffcbbell, .invc = 0xa3065e40u },
	{ .logc = -0x3b627b623312806all, .invc = 0xa16b312fu },
	{ .logc = -0x38dff78eac1ee139ll, .invc = 0x9fd809feu },
	{ .logc = -0x3663b0aeef9c794ell, .invc = 0x9e4cad24u },
	{ .logc = -0x33ed880e732cc827ll, .invc = 0x9cc8e161u },
	{ .logc = -0x317d5fd68bfd1855ll, .invc = 0x9b4c6f9fu },
	{ .logc = -0x2f131b0af698e67cll, .invc = 0x99d722dbu },
	{ .logc = -0x2cae9d7de42673e2ll, .invc = 0x9868c80au },
	{ .logc = -0x2a4fcbc8a76b19f4ll, .invc = 0x97012e02u },
	{ .logc = -0x27f68b4b22519714ll, .invc = 0x95a02568u },
	{ .logc = -0x25a2c218950273adll, .invc = 0x94458094u },
	{ .logc = -0x235456fc3fee53c7ll, .invc = 0x92f11384u },
	{ .logc = -0x210b316b86740d11ll, .invc = 0x91a2b3c5u },
	{ .logc = -0x1ec739829c111fccll, .invc = 0x905a3863u },
	{ .logc = -0x1c885801c04b2369ll, .invc = 0x8f1779dau },
	{ .logc = -0x1a4e76404dbc37a9ll, .invc = 0x8dda5202u },
	{ .logc = -0x18197e2e88e3f01bll, .invc = 0x8ca29c04u },
	{ .logc = -0x15e95a4d7391cb7dll, .invc = 0x8b70344au },
	{ .logc = -0x13bdf5a731ee642fll, .invc = 0x8a42f870u },
	{ .logc = -0x11973bd1705566d1ll, .invc = 0x891ac73bu },
	{ .logc = -0x0f7518e0135c3dd8ll, .invc = 0x87f78088u },
	{ .logc = -0x0d57796795887e0dll, .invc = 0x86d90544u },
	{ .logc = -0x0b3e4a79145dac20ll, .invc = 0x85bf3761u },
	{ .logc = -0x09297997b68c1f4dll, .invc = 0x84a9f9c8u },
	{ .logc = -0x0718f4ba892abc63ll, .invc = 0x83993052u },
	{ .logc = -0x050caa49f0033027ll, .invc = 0x828cbfbfu },
	{ .logc = -0x030489147b145544ll, .invc = 0x81848da9u },
	{ .logc = -0x010080569388b357ll, .invc = 0x80808081u },
	{ .logc = 0x00ff805615885e03ll, .invc = 0x7f807f80u },
	{ .logc = 0x02fb88ec00214edcll, .invc = 0x7e8472a8u },
	{ .logc = 0x04f3a90fd3a95d3cll, .invc = 0x7d8c42b3u },
	{ .logc = 0x06e7f008fde465ffll, .invc = 0x7c97d911u },
	{ .logc = 0x08d86cc4bfecbfe1ll, .invc = 0x7ba71fe1u },
	{ .logc = 0x0ac52dd7b2726a46ll, .invc = 0x7aba01ebu },
	{ .logc = 0x0cae41882871f5bfll, .invc = 0x79d06a96u },
	{ .logc = 0x0e93b5c65b85a909ll, .invc = 0x78ea45e7u },
	{ .logc = 0x10759835a8e47130ll, .invc = 0x78078078u },
	{ .logc = 0x1253f62df81416fall, .invc = 0x77280773u },
	{ .logc = 0x142edcbfae46f03cll, .invc = 0x764bc88cu },
	{ .logc = 0x160658a8db50c3b2ll, .invc = 0x7572b202u },
	{ .logc = 0x17da766d5b12cc84ll, .invc = 0x749cb290u },
	{ .logc = 0x19ab42464a33acdcll, .invc = 0x73c9b971u },
	{ .logc = 0x1b78c82bc0eda108ll, .invc = 0x72f9b658u },
	{ .logc = 0x1d4313d644b35d5ell, .invc = 0x722c996cu },
	{ .logc = 0x1f0a30c08962a662ll, .invc = 0x71625344u },
	{ .logc = 0x20ce2a24f6b2dc54ll, .invc = 0x709ad4e5u },
	{ .logc = 0x228f0b090a8558d1ll, .invc = 0x6fd60fbau },
	{ .logc = 0x244cde3260b596fbll, .invc = 0x6f13f596u },
	{ .logc = 0x2607ae32b0ffa5fell, .invc = 0x6e5478acu },
	{ .logc = 0x27bf8561cf8c5395ll, .invc = 0x6d978b8fu },
	{ .logc = 0x29746de816abcab5ll, .invc = 0x6cdd212bu },
	{ .logc = 0x2b2671b41e410ba7ll, .invc = 0x6c252cc7u },
	{ .logc = 0x2cd59a85a95aa1bell, .invc = 0x6b6fa1feu },
	{ .logc = 0x2e81f1eacc6f4993ll, .invc = 0x6abc74beu },
	{ .logc = 0x302b8141f4afd5adll, .invc = 0x6a0b9945u },
	{ .logc = 0x31d251bc2cda8155ll, .invc = 0x695d041eu },
	{ .logc = 0x33766c5d2df706acll, .invc = 0x68b0aa1fu },
	{ .logc = 0x3517d9f9205e3186ll, .invc = 0x68068068u },
	{ .logc = 0x36b6a33c536b48ddll, .invc = 0x675e7c5eu },
	{ .logc = 0x3852d0abea318146ll, .invc = 0x66b893a9u },
	{ .logc = 0x39ec6a9ca78bb892ll, .invc = 0x6614bc36u },
	{ .logc = 0x3b83794037d8fac2ll, .invc = 0x6572ec30u },
	{ .logc = 0x3d1804a668b4bfd3ll, .invc = 0x64d319feu },
	{ .logc = 0x3eaa14aca6f66e0ell, .invc = 0x64353c48u },
	{ .logc = 0x4039b1161d40731all, .invc = 0x639949ecu },
	{ .logc = 0x41c6e17e09643475ll, .invc = 0x62ff3a02u },
	{ .logc = 0x4351ad5e7add2c82ll, .invc = 0x626703d8u },
	{ .logc = 0x44da1c0a54a2c17bll, .invc = 0x61d09ef3u },
	{ .logc = 0x466034b6fc8dbd9dll, .invc = 0x613c030au },
	{ .logc = 0x47e3fe793e8bca36ll, .invc = 0x60a92806u },
	{ .logc = 0x4965804229d8e7bdll, .invc = 0x60180602u },
	{ .logc = 0x4ae4c0ebfc749277ll, .invc = 0x5f889545u },
	{ .logc = 0x4c61c7248f0613ebll, .invc = 0x5eface49u },
	{ .logc = 0x4ddc998a95616bc9ll, .invc = 0x5e6ea9afu },
	{ .logc = 0x4f553e982bdc3e1ell, .invc = 0x5de42046u },
	{ .logc = 0x50cbbca823a04ed6ll, .invc = 0x5d5b2b08u },
	{ .logc = 0x52401a0116274330ll, .invc = 0x5cd3c315u },
	{ .logc = 0x53b25ccabc09a6bell, .invc = 0x5c4de1b6u },
	{ .logc = 0x55228b0e1f498b3fll, .invc = 0x5bc9805cu },
	{ .logc = 0x5690aac3e73f8671ll, .invc = 0x5b46989au },
};
