#
# Copyright (C) 2026 Red Rocket Computing, LLC
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
# armv6s-m.mk
#
# Created on: Oct 19, 2026
#     Author: Stephen Street (stephen@redrocketcomputing.com)
#

# Armv6S-M (Cortex-M0, M0+, M1 with the OS extension): Armv6-M with SVC, no FPU (PLAN D37)

CROSS_FLAGS ?= -mthumb -march=armv6s-m -mfloat-abi=soft

export ARCH_FAMILY := arm
export LIBC_FPU := nofp
# <fenv.h> implementation: x86_64, fpv5-dp (every floating type in the FPU), or soft (D44)
export LIBC_FENV := soft
# libm layers: libc/math/<layer> replaces the generic files of the same name and may add helpers (D46, D55)
# nofp-thumb1 is the soft-float helpers that replace libgcc's one-function-per-object Armv6-M ones, listed first to
# keep the archive order they had inside nofp; nofp is the integer sqrtf and the speed flavour's fixed-point paths (D55, D85, D108)
# soft-int is the conversions to 64-bit integers, in integers (D102)
export LIBC_MATH_LAYERS := sp sp-nearest nofp-thumb1 nofp soft-int
# Hand-written string kernels (libc/string/<kernels>; the first directory with a function wins)
# per flavour: the speed build type sets LIBC_FLAVOUR (PLAN D69, D70, D73)
LIBC_FLAVOUR ?= size
LIBC_STRING_KERNELS_size := thumb1
LIBC_STRING_KERNELS_speed := thumb1-speed thumb1
export LIBC_STRING_KERNELS = ${LIBC_STRING_KERNELS_${LIBC_FLAVOUR}}

include ${PROJECT_ROOT}/tools/makefiles/arm-none-eabi.mk
