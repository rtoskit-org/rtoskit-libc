#
# Copyright (C) 2026 Red Rocket Computing, LLC
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
# x86_64.mk
#
# Created on: Oct 18, 2026
#     Author: Stephen Street (stephen@redrocketcomputing.com)
#

# x86_64 toolchain building against rtoskit-libc instead of the host C library

CROSS_FLAGS ?= -mlong-double-64

export ARCH_FAMILY := x86_64
# <fenv.h> implementation: x86_64, fpv5-dp (every floating type in the FPU), or soft (D44)
export LIBC_FENV := x86_64
# libm layers: libc/math/<layer> replaces the generic files of the same name and may add helpers (D46, D55)
export LIBC_MATH_LAYERS := x86_64
# Hand-written string kernels (libc/string/<kernels>; the first directory with a function wins)
# per flavour: the speed build type sets LIBC_FLAVOUR (PLAN D73, D76); the size flavour is the C code
LIBC_FLAVOUR ?= size

# The FPU for the private libm headers (PLAN D82): the SSE2 baseline (D81)
export LIBC_FPU := sse2
LIBC_STRING_KERNELS_size :=
LIBC_STRING_KERNELS_speed := x86_64
export LIBC_STRING_KERNELS = ${LIBC_STRING_KERNELS_${LIBC_FLAVOUR}}

# The host compiler: any GCC 14 or newer (PLAN D9, D107). tools/scripts/host-cc picks plain gcc when it is new
# enough, else the newest gcc-NN, and checks a HOST_CC given by the user. The result is exported with a marker so
# the sub-makes inherit it instead of running the check again.
ifeq (${HOST_CC},)
override HOST_CC := $(shell ${PROJECT_ROOT}/tools/scripts/host-cc)
else ifneq (${HOST_CC_CHECKED},${HOST_CC})
override HOST_CC := $(shell ${PROJECT_ROOT}/tools/scripts/host-cc)
endif
ifeq (${HOST_CC},)
$(error x86_64 needs GCC 14 or newer: install one, set HOST_CC, or build the Arm architectures alone with ARCH_CROSS or ARCHS)
endif
export HOST_CC
export HOST_CC_CHECKED := ${HOST_CC}

CC := ${HOST_CC}
CXX := $(subst gcc,g++,${HOST_CC})
LD := ${HOST_CC}
AR := ar
AS := as
OBJCOPY := objcopy
OBJDUMP := objdump
SIZE := size
NM := nm
INSTALL := install
INSTALL_STRIP := install --strip-program=strip -s

# The predefined feature macros a hosted compiler would take from <stdc-predef.h> (D50)
# libc/internal/fpu/${LIBC_FPU} holds the private libm header saying what the FPU computes (PLAN D82)
CPPFLAGS := -nostdinc -isystem ${PROJECT_ROOT}/include -include ${PROJECT_ROOT}/include/stdc-predef.h -I${PROJECT_ROOT}/libc/internal/fpu/${LIBC_FPU}
ARFLAGS := cr
ASFLAGS :=

# -mlong-double-64 makes long double binary64 as on the ARM targets (PLAN D7)
# -muclibc: the host compiler assumes glibc and would fuse sin and cos into sincos, which C does not
# reserve and rtoskit-libc does not provide; uClibc is the GNU/Linux C library it knows without sincos (D51)
# No unwind tables: C needs none, and .eh_frame was 28 % of libc.a (P2.02, D52)
# Frame sizes and call graphs for tools/scripts/size-report (D51)
CFLAGS := ${CROSS_FLAGS} -muclibc -fno-asynchronous-unwind-tables -fno-unwind-tables -pipe -feliminate-unused-debug-types -fmessage-length=0 -ffunction-sections -fdata-sections -fno-common -fno-pie -fno-stack-protector -Wall -Wunused -Wuninitialized -Wmissing-declarations -Werror -std=c23 -fstack-usage -fcallgraph-info=su
CXXFLAGS :=

# Added by the modules that build the library and the ports
LIBC_CFLAGS := -ffreestanding -fno-builtin -fno-tree-loop-distribute-patterns -ftls-model=local-exec

LDSCRIPTS :=
LDFLAGS := ${CROSS_FLAGS} -static -no-pie -nostdlib -Wl,--gc-sections
LOADLIBES :=
LDLIBS := -Wl,--start-group ${BUILD_ROOT}/libc/libc.a $(shell ${CC} ${CROSS_FLAGS} -print-libgcc-file-name) -Wl,--end-group

# As for host builds in release.mk: the native assembler cannot interleave source (-h) in listings of optimised code
LISTING_STYLE := -adlns
