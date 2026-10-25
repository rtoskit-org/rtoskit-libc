#
# Copyright (C) 2026 Red Rocket Computing, LLC
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
# armv7-m.mk
#
# Created on: Oct 25, 2026
#     Author: Stephen Street (stephen@redrocketcomputing.com)
#

# Armv7-M without an FPU or the DSP extension (Cortex-M3; a Cortex-M4 or M33 built without its FPU), soft-float ABI
# (PLAN D108, docs/research/armv7-m.md)

CROSS_FLAGS ?= -mthumb -march=armv7-m -mfloat-abi=soft

export ARCH_FAMILY := arm
export LIBC_FPU := nofp
# <fenv.h> implementation: x86_64, fpv5-dp (every floating type in the FPU), or soft (D44)
export LIBC_FENV := soft
# libm layers: libc/math/<layer> replaces the generic files of the same name and may add helpers (D46, D55);
# nofp is the integer sqrtf and the speed flavour's fixed-point paths (D85, D88); soft-sp the float helpers, which
# replace libgcc's grouped Thumb-2 objects whole (D109), so the Armv6-M nofp-thumb1 helpers are not listed (D108)
# soft-dp is the double-precision run-time ABI helpers in integers (D99), soft-dp-nodsp its multiplication
# without UMAAL (D108); soft-int is the conversions to 64-bit integers, in integers (D102)
export LIBC_MATH_LAYERS := sp sp-nearest nofp soft-sp soft-dp soft-dp-nodsp soft-int
# Hand-written string kernels (libc/string/<kernels>; the first directory with a function wins)
# per flavour: the speed build type sets LIBC_FLAVOUR (PLAN D69, D70, D73); no thumb2-dsp (D108)
LIBC_FLAVOUR ?= size
LIBC_STRING_KERNELS_size := thumb1
LIBC_STRING_KERNELS_speed := thumb2 thumb1
export LIBC_STRING_KERNELS = ${LIBC_STRING_KERNELS_${LIBC_FLAVOUR}}

include ${PROJECT_ROOT}/tools/makefiles/arm-none-eabi.mk
