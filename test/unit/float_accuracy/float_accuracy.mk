#
# Copyright (C) 2026 Red Rocket Computing, LLC
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
# float_accuracy.mk
#
# Created on: Oct 20, 2026
#     Author: Stephen Street (stephen@redrocketcomputing.com)
#

ifeq ($(findstring ${BUILD_ROOT},${CURDIR}),)
include ${PROJECT_ROOT}/tools/makefiles/target.mk
else

EXTRA_DEPS := ${SOURCE_DIR}/float_accuracy.mk ${SOURCE_DIR}/subdir.mk
EXTRA_ELF_DEPS := ${BUILD_ROOT}/libc/libc.a

TARGET_OBJ_LIBS := port/${RTOS_PORT} port/${RTOS_PORT}/${ARCH_FAMILY}

# On x86_64 the float-float layer of the single-precision cores is compiled into the test, so that
# millions of arguments run natively; elsewhere the test measures the library's own functions (PLAN D57).
# The release run takes most of a minute natively, so float_accuracy.timeout raises run-unit's limit
ifeq (${ARCH_FAMILY},x86_64)
SP_NEAREST_SRC := $(wildcard ${PROJECT_ROOT}/libc/math/sp-nearest/*.c)
EXTRA_OBJS := $(patsubst ${PROJECT_ROOT}/libc/math/sp-nearest/%.c,${CURDIR}/sp-nearest/%.o,${SP_NEAREST_SRC})
SAMPLES := $(if $(filter debug,${BUILD_TYPE}),65536,2097152)
else
SAMPLES := 8192
endif

include ${PROJECT_ROOT}/tools/makefiles/project.mk

# Exercise the library, not the compiler's built-in versions of the functions
CFLAGS += -fno-builtin
CPPFLAGS += -I${PROJECT_ROOT}/libc -DFLOAT_ACCURACY_SAMPLES=${SAMPLES}

${CURDIR}/sp-nearest/%.o: ${PROJECT_ROOT}/libc/math/sp-nearest/%.c ${EXTRA_DEPS}
	@echo "COMPILING ${<}"
	@mkdir -p $(dir $@)
	${CC} ${CPPFLAGS} ${CFLAGS} -MMD -MP -c -o $@ $<

-include ${EXTRA_OBJS:.o=.d}

all: ${CURDIR}/float_accuracy.elf

# The fast build type's expectation compiles into a subdirectory of the build directory (subdir.mk)
${CURDIR}/fast/expect.o: | ${CURDIR}/fast
${CURDIR}/fast:
	mkdir -p $@

endif
