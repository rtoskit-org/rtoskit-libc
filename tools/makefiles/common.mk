#
# Copyright (C) 2017 Red Rocket Computing, LLC
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
# commom.mk
#
# Created on: Mar 16, 2017
#     Author: Stephen Street (stephen@redrocketcomputing.com)
#

# The architecture fixes the toolchain, the flags and the library variants (PLAN D24, D106)
-include ${PROJECT_ROOT}/tools/makefiles/${ARCH_CROSS}.mk

# Programs the build links (tests, examples, size and benchmark programs) bind to the Linux test port;
# libc.a and every port archive are built regardless (PLAN D106)
export RTOS_PORT := linux
-include ${PROJECT_ROOT}/tools/makefiles/port-${RTOS_PORT}-${ARCH_FAMILY}.mk

# The allocator built into libc.a (PLAN D32, D54)
export LIBC_MALLOC ?= nano

# LIBC_UNWIND_TABLES=1 builds the library with unwind tables, so a backtrace (std::stacktrace, _Unwind_Backtrace)
# can pass through library frames such as qsort's callbacks (PLAN D128, D134). Off by default: the size baselines
# and budgets are without them. Run "make clean" after changing it, as for LIBC_MALLOC.
export LIBC_UNWIND_TABLES ?=
LIBC_CFLAGS += $(if $(filter 1,${LIBC_UNWIND_TABLES}),-funwind-tables)

# LIBC_EXTRA_CFLAGS is the parent build's: compiler flags appended after the library's own, for the C sources of
# libc.a, its flavour archives and the port archives (for example -mpoke-function-name on Arm). Not the assembler
# sources, the link or the test programs. Empty by default, and the size baselines and budgets are without it.
# Run "make clean" after changing it, as for LIBC_MALLOC (PLAN D160).
export LIBC_EXTRA_CFLAGS ?=
LIBC_CFLAGS += ${LIBC_EXTRA_CFLAGS}

# The fast flavour's libm files (PLAN D162, D168): the stems of the files in libc/math/fast/ and libc/math/<layer>/fast/
# when the build type's LIBC_FLAVOURS (speed.mk, fast.mk) has fast, else nothing. The precedence goes by layer first
# (D168): generic < fast/ < each layer of LIBC_MATH_LAYERS in order, and within a layer its plain files < speed/ <
# fast/. So a fast/ file replaces the generic file and its own layer's and earlier layers' files of the same stem, and
# never a later layer's (the fixed-point nofp/speed files keep their place on the cores without an FPU)
# A layer's fast/ directory applies only where the core's arch file lists the layer in LIBC_MATH_FAST_LAYERS (D168:
# sp-nearest/fast/ computes in float with fused multiply-adds, a gain on the single-precision FPU cores and a loss in
# soft float); the generic fast/ applies everywhere
export LIBC_MATH_FAST_LAYERS ?=
libc_math_fast_layers = $(filter ${LIBC_MATH_FAST_LAYERS},$(1))
LIBC_MATH_FAST_DIRS = $(if $(filter fast,${LIBC_FLAVOURS}),${PROJECT_ROOT}/libc/math/fast $(foreach layer,$(call libc_math_fast_layers,${LIBC_MATH_LAYERS}),${PROJECT_ROOT}/libc/math/${layer}/fast))
LIBC_MATH_FAST_STEMS = $(basename $(notdir $(wildcard $(foreach dir,${LIBC_MATH_FAST_DIRS},${dir}/*.c ${dir}/*.S))))
# The layers of LIBC_MATH_LAYERS from $(1) on, $(1) included
libc_math_from = $(if $(2),$(if $(filter $(1),$(firstword $(2))),$(2),$(call libc_math_from,$(1),$(wordlist 2,$(words $(2)),$(2)))))
# ... and after $(1)
libc_math_later = $(wordlist 2,$(words $(call libc_math_from,$(1),${LIBC_MATH_LAYERS})),$(call libc_math_from,$(1),${LIBC_MATH_LAYERS}))
# The stems of the fast/ files of layer $(1) and the layers after it, in the fast flavour
libc_math_fast_stems_from = $(if $(filter fast,${LIBC_FLAVOURS}),$(basename $(notdir $(wildcard $(foreach layer,$(call libc_math_fast_layers,$(call libc_math_from,$(1),${LIBC_MATH_LAYERS})),${PROJECT_ROOT}/libc/math/${layer}/fast/*.c ${PROJECT_ROOT}/libc/math/${layer}/fast/*.S)))))
# SRC $(1) less the files a fast/ file replaces: for the generic module ($(2) empty) every fast/ file's, for a module
# of layer $(2) (its plain files or its speed/) those of its own fast/ and the later layers'
libc_math_fast_filter = $(filter-out $(foreach stem,$(if $(2),$(call libc_math_fast_stems_from,$(2)),${LIBC_MATH_FAST_STEMS}),${SOURCE_DIR}/${stem}.c ${SOURCE_DIR}/${stem}.S),${1})
# The stems of the files of the layers $(1) (plain, speed/ in the speed and fast flavours, fast/ in fast), which
# a generic fast/ file or an earlier layer's fast/ file gives way to (D168)
libc_math_layer_stems = $(basename $(notdir $(wildcard $(foreach layer,$(1),${PROJECT_ROOT}/libc/math/${layer}/*.c ${PROJECT_ROOT}/libc/math/${layer}/*.S $(if $(filter speed,${LIBC_FLAVOURS}),${PROJECT_ROOT}/libc/math/${layer}/speed/*.c) $(if $(and $(filter fast,${LIBC_FLAVOURS}),$(call libc_math_fast_layers,${layer})),${PROJECT_ROOT}/libc/math/${layer}/fast/*.c)))))

-include ${PROJECT_ROOT}/tools/makefiles/${CHIP_FAMILY}.mk
-include ${PROJECT_ROOT}/tools/makefiles/${CHIP_TYPE}.mk
-include ${PROJECT_ROOT}/tools/makefiles/${BUILD_TYPE}.mk

