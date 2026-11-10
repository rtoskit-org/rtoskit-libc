/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * float_hard.c
 *
 * Created on: Oct 22, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include <fenv.h>
#include <math.h>
#include <stdio.h>

#include "float_hard.h"

/*
 * The hard cases of the float functions that round their double function's
 * result (PLAN D98, docs/research/float-hard-cases.md): every argument the
 * exhaustive search (tools/scripts/float-hard-cases) found whose double
 * result is a float halfway point while the exact value is not, with the
 * correctly rounded result and the side of it on which the exact value
 * lies. Each is checked in every rounding direction: to nearest the stored
 * result, upward and downward the neighbour the exact value lies toward or
 * the result itself, toward zero the smaller in magnitude. Built where the
 * float functions take that path: x86_64 and the double-precision FPU
 * (subdir.mk); the float-float layer of the other cores has its own
 * allowance (D57).
 */

struct hard_case {
	const char *name;
	float (*function)(float);
	float x;
	float result;
	int direction;
};

static const struct hard_case cases[] = {
	{ "acosf", acosf, 0x1.110b46p-26f, 0x1.921fb6p+0f, -1 },
	{ "acosf", acosf, 0x1.04c444p-12f, 0x1.920f6ap+0f, -1 },
	{ "acoshf", acoshf, 0x1.b121a6p+75f, 0x1.a9a3f2p+5f, -1 },
	{ "acoshf", acoshf, 0x1.6351d8p+94f, 0x1.08b512p+6f, -1 },
	{ "asinhf", asinhf, 0x1.bacb4ap+24f, 0x1.1e0696p+4f, -1 },
	{ "asinhf", asinhf, -0x1.bacb4ap+24f, -0x1.1e0696p+4f, +1 },
	{ "asinhf", asinhf, 0x1.b121a6p+75f, 0x1.a9a3f2p+5f, -1 },
	{ "asinhf", asinhf, -0x1.b121a6p+75f, -0x1.a9a3f2p+5f, +1 },
	{ "asinhf", asinhf, 0x1.6351d8p+94f, 0x1.08b512p+6f, -1 },
	{ "asinhf", asinhf, -0x1.6351d8p+94f, -0x1.08b512p+6f, +1 },
	{ "atanf", atanf, 0x1.1ad646p-4f, 0x1.1a6386p-4f, -1 },
	{ "atanf", atanf, -0x1.1ad646p-4f, -0x1.1a6386p-4f, +1 },
	{ "atanpif", atanpif, 0x1.d26a62p-1f, 0x1.e1a662p-3f, +1 },
	{ "atanpif", atanpif, -0x1.d26a62p-1f, -0x1.e1a662p-3f, -1 },
	{ "cosf", cosf, 0x1.3170fp+63f, 0x1.fe2976p-1f, +1 },
	{ "cosf", cosf, -0x1.3170fp+63f, 0x1.fe2976p-1f, +1 },
	{ "cosf", cosf, -0x1.2b9622p+67f, 0x1.f0285ep-1f, -1 },
	{ "cosf", cosf, 0x1.2b9622p+67f, 0x1.f0285ep-1f, -1 },
	{ "erfcf", erfcf, -0x1.d93ec4p-17f, 0x1.00010ap+0f, +1 },
	{ "exp10m1f", exp10m1f, 0x1.fafecp+3f, 0x1.8c880ap+52f, +1 },
	{ "exp2f", exp2f, 0x1.853a6ep-9f, 0x1.00870ap+0f, -1 },
	{ "exp2f", exp2f, -0x1.e7526ep-6f, 0x1.f58d62p-1f, -1 },
	{ "lgammaf", lgammaf, -0x1.ade594p-30f, 0x1.446ab2p+4f, -1 },
	{ "lgammaf", lgammaf, 0x1.f8a754p-9f, 0x1.63acc2p+2f, +1 },
	{ "log10f", log10f, 0x1.fddcf4p-98f, -0x1.d33a46p+4f, +1 },
	{ "log10p1f", log10p1f, -0x1.a3c2e6p-31f, -0x1.6c999ep-32f, +1 },
	{ "log10p1f", log10p1f, 0x1.34f8p-12f, 0x1.0c53cap-13f, +1 },
	{ "log1pf", log1pf, 0x1.0f1fd6p+3f, 0x1.1fcbcep+1f, +1 },
	{ "log1pf", log1pf, 0x1.800006p-21f, 0x1.7ffffep-21f, -1 },
	{ "log1pf", log1pf, -0x1.7ffffap-21f, -0x1.800002p-21f, -1 },
	{ "log1pf", log1pf, 0x1.200036p-17f, 0x1.1fffe6p-17f, -1 },
	{ "log1pf", log1pf, -0x1.1fffcap-17f, -0x1.20001ap-17f, -1 },
	{ "log1pf", log1pf, -0x1.1d9188p-9f, -0x1.1de14ap-9f, +1 },
	{ "log1pf", log1pf, 0x1.fb035ap-2f, 0x1.9bddc2p-2f, +1 },
	{ "log1pf", log1pf, 0x1.b121a6p+76f, 0x1.a9a3f2p+5f, -1 },
	{ "log1pf", log1pf, 0x1.6351d8p+95f, 0x1.08b512p+6f, -1 },
	{ "log2p1f", log2p1f, 0x1.7a13c6p+30f, 0x1.e90026p+4f, +1 },
	{ "log2p1f", log2p1f, 0x1.251c66p+38f, 0x1.318ffap+5f, +1 },
	{ "logf", logf, 0x1.2f1fd6p+3f, 0x1.1fcbcep+1f, +1 },
	{ "logf", logf, 0x1.bacb4ap+25f, 0x1.1e0696p+4f, -1 },
	{ "logf", logf, 0x1.b121a6p+76f, 0x1.a9a3f2p+5f, -1 },
	{ "logf", logf, 0x1.6351d8p+95f, 0x1.08b512p+6f, -1 },
	{ "logf", logf, 0x1.827a74p-7f, -0x1.1c2b1ep+2f, -1 },
	{ "sinf", sinf, -0x1.33333p+13f, 0x1.63f4bap-2f, +1 },
	{ "sinf", sinf, 0x1.33333p+13f, -0x1.63f4bap-2f, -1 },
	{ "sinhf", sinhf, -0x1.250bfep-11f, -0x1.250bfep-11f, -1 },
	{ "sinhf", sinhf, 0x1.250bfep-11f, 0x1.250bfep-11f, +1 },
	{ "tgammaf", tgammaf, 0x1.bd0d52p-48f, 0x1.268266p+47f, +1 },
	{ "tgammaf", tgammaf, 0x1.c0a8eap-48f, 0x1.242422p+47f, +1 },
};

/* The correctly rounded result in mode: the exact value lies between result and its neighbour toward direction */
static float expected(const struct hard_case *c, int mode)
{
	float neighbour = nextafterf(c->result, c->direction > 0 ? INFINITY : -INFINITY);
	float above = c->direction > 0 ? neighbour : c->result;
	float below = c->direction > 0 ? c->result : neighbour;

	switch (mode) {
		case FE_UPWARD:
			return above;
		case FE_DOWNWARD:
			return below;
		case FE_TOWARDZERO:
			return fabsf(above) < fabsf(below) ? above : below;
		default:
			return c->result;
	}
}

int main(void)
{
	static const int modes[] = { FE_TONEAREST, FE_UPWARD, FE_DOWNWARD, FE_TOWARDZERO };
	static const char *const names[] = { "nearest", "upward", "downward", "toward zero" };
	int failures = 0;

	for (unsigned int m = 0; m < sizeof(modes) / sizeof(modes[0]); ++m) {
		for (unsigned int i = 0; i < sizeof(cases) / sizeof(cases[0]); ++i) {
			const struct hard_case *c = &cases[i];
			float want = expected(c, modes[m]);
			float got;

			fesetround(modes[m]);
			feclearexcept(FE_ALL_EXCEPT);
			got = c->function(c->x);
			int inexact = fetestexcept(FE_INEXACT);

			fesetround(FE_TONEAREST);
			if (!float_hard_accept(c->name, modes[m] == FE_TONEAREST, got, want, c->result, c->direction) || !inexact) {
				printf("%s(%a) %s: %a, want %a%s\n", c->name, (double)c->x, names[m], (double)got, (double)want, inexact ? "" : " (no inexact)");
				++failures;
			}
		}
	}
	printf("float_hard: %d of %u wrong\n", failures, (unsigned int)(4 * sizeof(cases) / sizeof(cases[0])));
	return failures != 0;
}
