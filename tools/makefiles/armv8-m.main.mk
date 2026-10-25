#
# Copyright (C) 2026 Red Rocket Computing, LLC
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
# armv8-m.main.mk
#
# Created on: Oct 19, 2026
#     Author: Stephen Street (stephen@redrocketcomputing.com)
#

# Armv8-M Mainline with DSP and the single-precision FPv5-SP-D16 (Cortex-M33, M35P), hard-float ABI

CROSS_FLAGS ?= -mthumb -march=armv8-m.main+fp+dsp -mfloat-abi=hard

export ARCH_FAMILY := arm
export LIBC_FPU := fpv5-sp
# <fenv.h> implementation: x86_64, fpv5-dp (every floating type in the FPU), or soft (D44)
export LIBC_FENV := soft
# libm layers: libc/math/<layer> replaces the generic files of the same name and may add helpers (D46, D55);
# soft-dp is the double-precision run-time ABI helpers in integers (D99)
# soft-int is the conversions to 64-bit integers, in integers (D102)
export LIBC_MATH_LAYERS := sp sp-nearest fpv5-sp soft-dp soft-int
# Hand-written string kernels (libc/string/<kernels>; the first directory with a function wins)
# per flavour: the speed build type sets LIBC_FLAVOUR (PLAN D69, D70, D73)
LIBC_FLAVOUR ?= size
LIBC_STRING_KERNELS_size := thumb1
LIBC_STRING_KERNELS_speed := thumb2-dsp thumb2 thumb1
export LIBC_STRING_KERNELS = ${LIBC_STRING_KERNELS_${LIBC_FLAVOUR}}

include ${PROJECT_ROOT}/tools/makefiles/arm-none-eabi.mk
