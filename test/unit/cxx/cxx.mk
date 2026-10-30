#
# Copyright (C) 2026 Red Rocket Computing, LLC
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
# cxx.mk
#
# Created on: Oct 30, 2026
#     Author: Stephen Street (stephen@redrocketcomputing.com)
#

ifeq ($(findstring ${BUILD_ROOT},${CURDIR}),)
include ${PROJECT_ROOT}/tools/makefiles/target.mk
else

EXTRA_DEPS := ${SOURCE_DIR}/cxx.mk
EXTRA_ELF_DEPS := ${BUILD_ROOT}/libc/libc.a

TARGET_OBJ_LIBS := port/${RTOS_PORT} port/${RTOS_PORT}/${ARCH_FAMILY}

include ${PROJECT_ROOT}/tools/makefiles/project.mk

# The C++ runtime is the toolchain's libsupc++ (exceptions, RTTI, operator new); every C library function, and the
# C++ ABI hooks the C library owns (__cxa_atexit, __cxa_thread_atexit), come from libc.a, which the group lists
# first (PLAN D121). The test includes only rtoskit-libc's headers, so -nostdinc stays.
CXXFLAGS += ${CROSS_FLAGS} -std=c++23 -pipe -ffunction-sections -fdata-sections -Wall -Wextra -Werror -fno-builtin
${CURDIR}/headers17.o: CXXFLAGS += -std=c++17
LDLIBS := -Wl,--start-group ${BUILD_ROOT}/libc/libc.a $(shell ${CXX} ${CROSS_FLAGS} -print-file-name=libsupc++.a) $(shell ${CXX} ${CROSS_FLAGS} -print-file-name=libstdc++.a) $(shell ${CC} ${CROSS_FLAGS} -print-libgcc-file-name) -Wl,--end-group

all: $(if ${SRC},${CURDIR}/cxx.elf)

endif
