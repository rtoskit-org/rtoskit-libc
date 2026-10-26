#
# Copyright (C) 2026 Red Rocket Computing, LLC
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
# malloc.mk
#
# Created on: Oct 18, 2026
#     Author: Stephen Street (stephen@redrocketcomputing.com)
#

ifeq ($(findstring ${BUILD_ROOT},${CURDIR}),)
include ${PROJECT_ROOT}/tools/makefiles/target.mk
else

EXTRA_DEPS := ${SOURCE_DIR}/malloc.mk
EXTRA_ELF_DEPS := ${BUILD_ROOT}/libc/libc.a

TARGET_OBJ_LIBS := port/${RTOS_PORT} port/${RTOS_PORT}/${ARCH_FAMILY}

include ${PROJECT_ROOT}/tools/makefiles/project.mk

# Exercise the library, not the compiler's built-in versions of the functions
CFLAGS += -fno-builtin

# The heap consistency checker is an internal libc service
CPPFLAGS += -I${PROJECT_ROOT}/libc

# One executable per allocator variant: its objects come first, so libc.a's allocator is never pulled in (PLAN D32)
MALLOC_VARIANTS := nano tlsf tiny
MALLOC_COMMON_OBJ := $(call module_objects,${BUILD_ROOT}/libc/malloc)

all: $(foreach variant,${MALLOC_VARIANTS},${CURDIR}/malloc-${variant}.elf)

define malloc_variant
${CURDIR}/malloc-$(1).elf: ${OBJ} ${TARGET_OBJ} $(call module_objects,${BUILD_ROOT}/libc/malloc/$(1)) ${MALLOC_COMMON_OBJ} ${EXTRA_ELF_DEPS}
	@echo "LINKING $$@"
	$$(LD) $${LDFLAGS} -Wl,-Map,"$$(basename $$@).map" $${LOADLIBES} -o $$@ $${OBJ} $(call module_objects,${BUILD_ROOT}/libc/malloc/$(1)) $${MALLOC_COMMON_OBJ} $${TARGET_OBJ} $${LDLIBS}
endef
$(foreach variant,${MALLOC_VARIANTS},$(eval $(call malloc_variant,${variant})))

endif
