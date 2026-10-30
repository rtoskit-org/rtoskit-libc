#
# Copyright (C) 2026 Red Rocket Computing, LLC
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
# api.mk
#
# Created on: Oct 30, 2026
#     Author: Stephen Street (stephen@redrocketcomputing.com)
#

ifeq ($(findstring ${BUILD_ROOT},${CURDIR}),)
include ${PROJECT_ROOT}/tools/makefiles/target.mk
else

EXTRA_DEPS := ${SOURCE_DIR}/api.mk
EXTRA_ELF_DEPS := ${BUILD_ROOT}/libc/libc.a

TARGET_OBJ_LIBS := port/${RTOS_PORT} port/${RTOS_PORT}/${ARCH_FAMILY}

include ${PROJECT_ROOT}/tools/makefiles/project.mk

# Exercise the library, not the compiler's built-in versions of the functions
CFLAGS += -fno-builtin

# Every public header compiles alone in strict C23 (issue #93): -pedantic-errors rejects any extension a header relies
# on without the __extension__ or reserved spelling that marks it. One stamp per header, rebuilt when the header changes.
API_HEADERS := $(wildcard ${PROJECT_ROOT}/include/*.h) ${PROJECT_ROOT}/include/rtos/rtos-port.h
API_STAMPS := $(patsubst ${PROJECT_ROOT}/include/%.h,${CURDIR}/standalone/%.ok,${API_HEADERS})

${CURDIR}/standalone/%.ok: ${PROJECT_ROOT}/include/%.h
	@echo "CHECKING $<"
	@mkdir -p $(dir $@)
	echo 'typedef int api_unit; /* a translation unit may not be empty (C23 6.9) */' | $(CC) ${CPPFLAGS} ${CFLAGS} -pedantic-errors -Werror -fsyntax-only -x c -include $< -
	@touch $@

all: ${CURDIR}/api.elf ${API_STAMPS}

endif
