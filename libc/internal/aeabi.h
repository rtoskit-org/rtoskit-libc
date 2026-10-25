/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * aeabi.h
 *
 * Created on: Oct 20, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#ifndef _INTERNAL_AEABI_H
#define _INTERNAL_AEABI_H

/*
 * The soft-float helpers of the Arm run-time ABI (RTABI 4.1.2), which the
 * compiler calls for floating-point arithmetic its FPU lacks. The
 * compiler's library provides them; this library replaces some:
 * - the nofp layer, on the cores without an FPU: the two subtractions
 *   (PLAN D55), the single-precision multiplication and division (D85),
 *   and the unsigned conversions to int (D102);
 * - the soft-dp layer, on the single-precision FPUs: the double-precision
 *   set (D99);
 * - the soft-int layer, on both: the conversions to 64-bit integers (D102).
 * They keep the base procedure call standard (RTABI 4.1.1) even in a
 * hard-float build.
 */
#define __LIBC_AEABI [[gnu::pcs("aapcs")]]

__LIBC_AEABI double __aeabi_dadd(double a, double b);
__LIBC_AEABI double __aeabi_dsub(double a, double b);
__LIBC_AEABI double __aeabi_dmul(double a, double b);
__LIBC_AEABI double __aeabi_ddiv(double a, double b);
__LIBC_AEABI int __aeabi_dcmpeq(double a, double b);
__LIBC_AEABI int __aeabi_dcmplt(double a, double b);
__LIBC_AEABI int __aeabi_dcmple(double a, double b);
__LIBC_AEABI int __aeabi_dcmpge(double a, double b);
__LIBC_AEABI int __aeabi_dcmpgt(double a, double b);
__LIBC_AEABI int __aeabi_dcmpun(double a, double b);
__LIBC_AEABI float __aeabi_d2f(double a);
__LIBC_AEABI double __aeabi_f2d(float a);
__LIBC_AEABI double __aeabi_i2d(int a);
__LIBC_AEABI double __aeabi_ui2d(unsigned int a);
__LIBC_AEABI double __aeabi_l2d(long long a);
__LIBC_AEABI double __aeabi_ul2d(unsigned long long a);
__LIBC_AEABI int __aeabi_d2iz(double a);
__LIBC_AEABI unsigned int __aeabi_d2uiz(double a);
__LIBC_AEABI unsigned int __aeabi_f2uiz(float a);
__LIBC_AEABI long long __aeabi_d2lz(double a);
__LIBC_AEABI unsigned long long __aeabi_d2ulz(double a);
__LIBC_AEABI long long __aeabi_f2lz(float a);
__LIBC_AEABI unsigned long long __aeabi_f2ulz(float a);
__LIBC_AEABI float __aeabi_fadd(float a, float b);
__LIBC_AEABI float __aeabi_fsub(float a, float b);
__LIBC_AEABI float __aeabi_fmul(float a, float b);
__LIBC_AEABI float __aeabi_fdiv(float a, float b);
__LIBC_AEABI float __aeabi_frsub(float a, float b);
__LIBC_AEABI int __aeabi_fcmpeq(float a, float b);
__LIBC_AEABI int __aeabi_fcmplt(float a, float b);
__LIBC_AEABI int __aeabi_fcmple(float a, float b);
__LIBC_AEABI int __aeabi_fcmpge(float a, float b);
__LIBC_AEABI int __aeabi_fcmpgt(float a, float b);
__LIBC_AEABI float __aeabi_i2f(int a);
__LIBC_AEABI float __aeabi_ui2f(unsigned int a);
__LIBC_AEABI float __aeabi_l2f(long long a);
__LIBC_AEABI float __aeabi_ul2f(unsigned long long a);

#endif /* _INTERNAL_AEABI_H */
