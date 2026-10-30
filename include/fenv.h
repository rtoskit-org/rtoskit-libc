/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * fenv.h
 *
 * Created on: Oct 19, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#ifndef _FENV_H
#define _FENV_H

#include <__cdefs.h>

__LIBC_BEGIN_DECLS

/*
 * C23 7.6 (docs/research/fenv-libm.md, PLAN D44). The values are the same
 * for every target: the exception flags and rounding directions use the Arm
 * FPSCR encoding, which the x86_64 implementation translates to MXCSR.
 */
#define __STDC_VERSION_FENV_H__ 202311L

/* The whole floating-point state register (MXCSR or FPSCR) */
typedef struct {
	unsigned int __state;
} fenv_t;

/* Its control part: everything but the exception flags */
typedef struct {
	unsigned int __mode;
} femode_t;

typedef unsigned int fexcept_t;

/*
 * 7.6p6, p8: an exception or rounding macro is defined only when it is
 * supported. Exceptions and directed rounding exist only where every
 * floating type is implemented by the FPU (x86_64 SSE, Arm with a double-
 * precision FPU); elsewhere double (or all) arithmetic is libgcc's software
 * floating point, which rounds to nearest and records no exceptions.
 */
#if defined(__x86_64__) || (defined(__ARM_FP) && (__ARM_FP & 8) != 0)
#define FE_INVALID 0x01
#define FE_DIVBYZERO 0x02
#define FE_OVERFLOW 0x04
#define FE_UNDERFLOW 0x08
#define FE_INEXACT 0x10
#define FE_ALL_EXCEPT (FE_INVALID | FE_DIVBYZERO | FE_OVERFLOW | FE_UNDERFLOW | FE_INEXACT)

#define FE_TONEAREST 0
#define FE_UPWARD 1
#define FE_DOWNWARD 2
#define FE_TOWARDZERO 3
#else
#define FE_ALL_EXCEPT 0
#define FE_TONEAREST 0
#endif

/* The state at program start-up */
#define FE_DFL_ENV ((const fenv_t *)-1)
#define FE_DFL_MODE ((const femode_t *)-1)

int feclearexcept(int excepts);
int fegetexceptflag(fexcept_t *flagp, int excepts);
int feraiseexcept(int excepts);
int fesetexcept(int excepts);
int fesetexceptflag(const fexcept_t *flagp, int excepts);
int fetestexceptflag(const fexcept_t *flagp, int excepts);
int fetestexcept(int excepts);
int fegetmode(femode_t *modep);
int fegetround(void);
int fesetmode(const femode_t *modep);
int fesetround(int rnd);
int fegetenv(fenv_t *envp);
int feholdexcept(fenv_t *envp);
int fesetenv(const fenv_t *envp);
int feupdateenv(const fenv_t *envp);

__LIBC_END_DECLS

#endif /* _FENV_H */
