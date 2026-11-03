#
# Copyright (C) 2026 Red Rocket Computing, LLC
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
# unwind.mk
#
# Created on: Nov 3, 2026
#     Author: Stephen Street (stephen@redrocketcomputing.com)
#

ifeq ($(findstring ${BUILD_ROOT},${CURDIR}),)
include ${PROJECT_ROOT}/tools/makefiles/target.mk
else

EXTRA_DEPS := ${SOURCE_DIR}/unwind.mk
EXTRA_ELF_DEPS := ${BUILD_ROOT}/libc/libc.a

TARGET_OBJ_LIBS := port/${RTOS_PORT} port/${RTOS_PORT}/${ARCH_FAMILY}

include ${PROJECT_ROOT}/tools/makefiles/project.mk

# Exercise the library, not the compiler's built-in versions of the functions
CFLAGS += -fno-builtin

# The test's own frames carry unwind tables; the library's do only in a LIBC_UNWIND_TABLES=1 build (PLAN D134).
# <unwind.h> is the compiler's, searched after the library's headers.
CFLAGS += -funwind-tables
CPPFLAGS += -idirafter $(shell ${CC} ${CROSS_FLAGS} -print-file-name=include)
CPPFLAGS += -DUNWIND_LIBRARY=$(if $(filter 1,${LIBC_UNWIND_TABLES}),1,0)

all: $(if ${SRC},${CURDIR}/unwind.elf)

endif
