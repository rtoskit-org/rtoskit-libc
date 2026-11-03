#
# Copyright (C) 2026 Red Rocket Computing, LLC
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
# freestanding.mk
#
# Created on: Oct 19, 2026
#     Author: Stephen Street (stephen@redrocketcomputing.com)
#

ifeq ($(findstring ${BUILD_ROOT},${CURDIR}),)
include ${PROJECT_ROOT}/tools/makefiles/target.mk
else

EXTRA_DEPS := ${SOURCE_DIR}/freestanding.mk
EXTRA_ELF_DEPS := ${BUILD_ROOT}/libc/libc.a

TARGET_OBJ_LIBS := port/${RTOS_PORT} port/${RTOS_PORT}/${ARCH_FAMILY}

include ${PROJECT_ROOT}/tools/makefiles/project.mk

# The library, not the compiler's built-in versions
CFLAGS += -fno-builtin

all: ${CURDIR}/freestanding.elf ${CURDIR}/freestanding-bare.elf

# Linked with libc.a alone (no port, no startup; check() is the entry): fails if a freestanding function needs the RTOS seam.
# A LIBC_UNWIND_TABLES=1 library's unwind entries reference libgcc's EHABI personality routine, which calls abort, so
# that build needs __rtos_abort and nothing else from the seam (PLAN D134); the stand-in is never called.
comma := ,
${CURDIR}/freestanding-bare.elf: ${CURDIR}/freestanding.o ${EXTRA_ELF_DEPS}
	@echo "LINKING ${@}"
	$(LD) ${LDFLAGS} -Wl,-e,check $(if $(filter 1,${LIBC_UNWIND_TABLES}),-Wl${comma}--defsym=__rtos_abort=check) -o $@ $< ${LDLIBS}

endif
