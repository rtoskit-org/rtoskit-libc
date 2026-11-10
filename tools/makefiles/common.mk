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

# The fast flavour's libm files (PLAN D162): the stems of the files in libc/math/fast/ and libc/math/<layer>/fast/
# when the build type's LIBC_FLAVOURS (speed.mk, fast.mk) has fast, else nothing. A fast/ file replaces the generic,
# layer and speed/ file of the same stem, so each of those modules' subdir.mk drops them from its SRC
LIBC_MATH_FAST_DIRS = $(if $(filter fast,${LIBC_FLAVOURS}),${PROJECT_ROOT}/libc/math/fast $(foreach layer,${LIBC_MATH_LAYERS},${PROJECT_ROOT}/libc/math/${layer}/fast))
LIBC_MATH_FAST_STEMS = $(basename $(notdir $(wildcard $(foreach dir,${LIBC_MATH_FAST_DIRS},${dir}/*.c ${dir}/*.S))))
libc_math_fast_filter = $(filter-out $(foreach stem,${LIBC_MATH_FAST_STEMS},${SOURCE_DIR}/${stem}.c ${SOURCE_DIR}/${stem}.S),${1})

-include ${PROJECT_ROOT}/tools/makefiles/${CHIP_FAMILY}.mk
-include ${PROJECT_ROOT}/tools/makefiles/${CHIP_TYPE}.mk
-include ${PROJECT_ROOT}/tools/makefiles/${BUILD_TYPE}.mk

