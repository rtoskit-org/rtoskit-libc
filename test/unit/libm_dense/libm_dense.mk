#
# Copyright (C) 2026 Red Rocket Computing, LLC
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
# libm_dense.mk
#
# Created on: Oct 21, 2026
#     Author: Stephen Street (stephen@redrocketcomputing.com)
#

ifeq ($(findstring ${BUILD_ROOT},${CURDIR}),)
include ${PROJECT_ROOT}/tools/makefiles/target.mk
else

EXTRA_DEPS := ${SOURCE_DIR}/libm_dense.mk ${SOURCE_DIR}/subdir.mk ${SOURCE_DIR}/near.inc
EXTRA_ELF_DEPS := ${BUILD_ROOT}/libc/libc.a

TARGET_OBJ_LIBS := port/${RTOS_PORT} port/${RTOS_PORT}/${ARCH_FAMILY}

include ${PROJECT_ROOT}/tools/makefiles/project.mk

# Exercise the library, not the compiler's built-in versions of the functions
CFLAGS += -fno-builtin

all: ${CURDIR}/libm_dense.elf

# The fast build type's expectation compiles into a subdirectory of the build directory (subdir.mk)
${CURDIR}/fast/expect.o: | ${CURDIR}/fast
${CURDIR}/fast:
	mkdir -p $@

endif
