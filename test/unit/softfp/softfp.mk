#
# Copyright (C) 2026 Red Rocket Computing, LLC
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
# softfp.mk
#
# Created on: Oct 20, 2026
#     Author: Stephen Street (stephen@redrocketcomputing.com)
#

ifeq ($(findstring ${BUILD_ROOT},${CURDIR}),)
include ${PROJECT_ROOT}/tools/makefiles/target.mk
else

EXTRA_DEPS := ${SOURCE_DIR}/softfp.mk
EXTRA_ELF_DEPS := ${BUILD_ROOT}/libc/libc.a

TARGET_OBJ_LIBS := port/${RTOS_PORT} port/${RTOS_PORT}/${ARCH_FAMILY}

# On x86_64 the integer single-precision helpers of the nofp-thumb1 layer are compiled into the test, so that the
# comparison with the hardware runs natively (PLAN D85)
ifeq (${ARCH_FAMILY},x86_64)
SOFTFLOAT_SRC := ${PROJECT_ROOT}/libc/math/nofp-thumb1/__aeabi_fmul.c ${PROJECT_ROOT}/libc/math/nofp-thumb1/__aeabi_fdiv.c
EXTRA_OBJS := $(patsubst ${PROJECT_ROOT}/libc/math/nofp-thumb1/%.c,${CURDIR}/nofp-thumb1/%.o,${SOFTFLOAT_SRC})
endif

include ${PROJECT_ROOT}/tools/makefiles/project.mk

# Exercise the library, not the compiler's built-in versions of the functions
CFLAGS += -fno-builtin

# The helpers are internal to the library; their base calling convention attribute is Arm's only
CPPFLAGS += -I${PROJECT_ROOT}/libc -I${PROJECT_ROOT}/libc/math/nofp
CFLAGS += -Wno-attributes


${CURDIR}/nofp-thumb1/%.o: ${PROJECT_ROOT}/libc/math/nofp-thumb1/%.c ${EXTRA_DEPS}
	@echo "COMPILING ${<}"
	@mkdir -p $(dir $@)
	${CC} ${CPPFLAGS} ${CFLAGS} -MMD -MP -c -o $@ $<

-include ${EXTRA_OBJS:.o=.d}

all: ${CURDIR}/softfp.elf

endif
