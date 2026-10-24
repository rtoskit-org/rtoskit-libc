#
# Copyright (C) 2026 Red Rocket Computing, LLC
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
# arm-none-eabi.mk
#
# Created on: Oct 19, 2026
#     Author: Stephen Street (stephen@redrocketcomputing.com)
#

# Shared part of the Arm M-profile toolchain files: the architecture file sets
# CROSS_FLAGS, ARCH_FAMILY and LIBC_FPU, then includes this file (PLAN D22, D24, D34).
# The port file common.mk includes next appends its link options (PLAN D106).

CROSS_COMPILE ?= ${PROJECT_ROOT}/local/bin/arm-none-eabi-

CC := ${CROSS_COMPILE}gcc
CXX := ${CROSS_COMPILE}g++
LD := ${CROSS_COMPILE}gcc
AR := ${CROSS_COMPILE}ar
AS := ${CROSS_COMPILE}as
OBJCOPY := ${CROSS_COMPILE}objcopy
OBJDUMP := ${CROSS_COMPILE}objdump
SIZE := ${CROSS_COMPILE}size
NM := ${CROSS_COMPILE}nm
INSTALL := install
INSTALL_STRIP := install --strip-program=${CROSS_COMPILE}strip -s

# The predefined feature macros a hosted compiler would take from <stdc-predef.h> (D50)
# libc/internal/fpu/${LIBC_FPU} holds the private libm header saying what the FPU computes (PLAN D82)
CPPFLAGS := -nostdinc -isystem ${PROJECT_ROOT}/include -include ${PROJECT_ROOT}/include/stdc-predef.h -I${PROJECT_ROOT}/libc/internal/fpu/${LIBC_FPU}
ARFLAGS := cr

# Assembler sources are preprocessed by the compiler driver, which needs the target selection too
ASFLAGS := ${CROSS_FLAGS}

# Frame sizes and call graphs for tools/scripts/size-report (D51)
CFLAGS := ${CROSS_FLAGS} -pipe -feliminate-unused-debug-types -fmessage-length=0 -ffunction-sections -fdata-sections -fno-common -Wall -Wunused -Wuninitialized -Wmissing-declarations -Werror -std=c23 -fstack-usage -fcallgraph-info=su
CXXFLAGS :=

# Added by the modules that build the library and the ports
LIBC_CFLAGS := -ffreestanding -fno-builtin -fno-tree-loop-distribute-patterns -ftls-model=local-exec

LDSCRIPTS :=
# Bare-metal objects carry no .note.GNU-stack; the stack is never executable, so say so instead of letting ld warn
LDFLAGS := ${CROSS_FLAGS} -static -nostdlib -Wl,--gc-sections -Wl,-z,noexecstack
LOADLIBES :=
LDLIBS := -Wl,--start-group ${BUILD_ROOT}/libc/libc.a $(shell ${CC} ${CROSS_FLAGS} -print-libgcc-file-name) -Wl,--end-group
