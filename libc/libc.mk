#
# Copyright (C) 2026 Red Rocket Computing, LLC
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
# libc.mk
#
# Created on: Oct 18, 2026
#     Author: Stephen Street (stephen@redrocketcomputing.com)
#

ifeq ($(findstring ${BUILD_ROOT},${CURDIR}),)
include ${PROJECT_ROOT}/tools/makefiles/target.mk
else

# libc.a collects the objects of every libc module, including the ${ARCH_CROSS} and ${ARCH_FAMILY} variants
# and the allocator variant named by ${LIBC_MALLOC} (set by the board file, PLAN D32)
$(if $(wildcard ${PROJECT_ROOT}/libc/malloc/${LIBC_MALLOC}/${LIBC_MALLOC}.mk),,$(error LIBC_MALLOC='${LIBC_MALLOC}' is not an allocator in libc/malloc))
TARGET_OBJ_LIBS := libc/assert
TARGET_OBJ_LIBS += libc/atomic
TARGET_OBJ_LIBS += libc/complex
TARGET_OBJ_LIBS += $(addprefix libc/complex/,$(filter sp-nearest,${LIBC_MATH_LAYERS}))
TARGET_OBJ_LIBS += libc/ctype
TARGET_OBJ_LIBS += libc/decimal
TARGET_OBJ_LIBS += libc/errno
TARGET_OBJ_LIBS += libc/fenv
TARGET_OBJ_LIBS += libc/fenv/${LIBC_FENV}
TARGET_OBJ_LIBS += libc/inttypes
TARGET_OBJ_LIBS += libc/locale
TARGET_OBJ_LIBS += libc/malloc
TARGET_OBJ_LIBS += libc/malloc/${LIBC_MALLOC}
TARGET_OBJ_LIBS += libc/math
TARGET_OBJ_LIBS += $(addprefix libc/math/,${LIBC_MATH_LAYERS})
TARGET_OBJ_LIBS += libc/setjmp/${ARCH_CROSS}
TARGET_OBJ_LIBS += libc/signal
TARGET_OBJ_LIBS += libc/startup
TARGET_OBJ_LIBS += libc/stdbit
TARGET_OBJ_LIBS += libc/stdio
TARGET_OBJ_LIBS += libc/stdlib
# The speed flavour's replacement files (PLAN D77)
TARGET_OBJ_LIBS += $(if $(filter speed,${LIBC_FLAVOUR}),libc/stdio/speed libc/stdlib/speed)
TARGET_OBJ_LIBS += $(if $(filter speed,${LIBC_FLAVOUR}),$(foreach layer,${LIBC_MATH_LAYERS},$(if $(wildcard ${PROJECT_ROOT}/libc/math/${layer}/speed/speed.mk),libc/math/${layer}/speed)))
TARGET_OBJ_LIBS += libc/string
TARGET_OBJ_LIBS += $(addprefix libc/string/,${LIBC_STRING_KERNELS})
TARGET_OBJ_LIBS += libc/threads
TARGET_OBJ_LIBS += libc/time
TARGET_OBJ_LIBS += libc/tls/${ARCH_FAMILY}

EXTRA_DEPS := ${SOURCE_DIR}/libc.mk

LIBC_HEADERS := $(patsubst ${PROJECT_ROOT}/include/%,${INSTALL_ROOT}/include/%,$(shell find ${PROJECT_ROOT}/include -name '*.h'))

include ${PROJECT_ROOT}/tools/makefiles/project.mk

all: ${INSTALL_ROOT}/lib/libc.a ${INSTALL_ROOT}/lib/libm.a ${LIBC_HEADERS}
all: ${INSTALL_ROOT}/lib/libc-nofloat.a ${INSTALL_ROOT}/lib/libc-minimal-printf.a ${INSTALL_ROOT}/lib/libc-tinymalloc.a

# Flavour archives (PLAN D13, D53): libc.a with members replaced by the objects of a variant
# module of the same name, chosen by linking the archive instead of libc.a
LIBC_NOFLOAT_OBJ := $(call module_objects,${BUILD_ROOT}/libc/stdio/nofloat)
LIBC_MINIMAL_OBJ := ${LIBC_NOFLOAT_OBJ} $(call module_objects,${BUILD_ROOT}/libc/stdio/minimal)
LIBC_TINYMALLOC_OBJ := $(call module_objects,${BUILD_ROOT}/libc/malloc/tiny)
libc_flavour = $(filter-out $(addprefix %/,$(notdir ${1})),${TARGET_OBJ}) ${1}

${CURDIR}/libc-nofloat.a: ${TARGET_OBJ} ${LIBC_NOFLOAT_OBJ} ${EXTRA_DEPS}
	@echo "ARCHIVING ${@}"
	${RM} $@
	$(AR) ${ARFLAGS} $@ $(call libc_flavour,${LIBC_NOFLOAT_OBJ})

${CURDIR}/libc-minimal-printf.a: ${TARGET_OBJ} ${LIBC_MINIMAL_OBJ} ${EXTRA_DEPS}
	@echo "ARCHIVING ${@}"
	${RM} $@
	$(AR) ${ARFLAGS} $@ $(call libc_flavour,${LIBC_MINIMAL_OBJ})

${CURDIR}/libc-tinymalloc.a: ${TARGET_OBJ} ${LIBC_TINYMALLOC_OBJ} ${EXTRA_DEPS}
	@echo "ARCHIVING ${@}"
	${RM} $@
	$(AR) ${ARFLAGS} $@ $(call libc_flavour,${LIBC_TINYMALLOC_OBJ})

${INSTALL_ROOT}/lib/libc-%.a: ${CURDIR}/libc-%.a
	@echo "INSTALLING ${@}"
	mkdir -p $(@D)
	$(INSTALL) -m 644 ${<} ${@}

# libm is part of libc.a (D46); an empty libm.a keeps "-lm" working
${CURDIR}/libm.a: ${EXTRA_DEPS}
	@echo "ARCHIVING ${@}"
	rm -f $@
	$(AR) ${ARFLAGS} $@

${INSTALL_ROOT}/lib/libm.a: ${CURDIR}/libm.a
	@echo "INSTALLING ${@}"
	mkdir -p $(@D)
	$(INSTALL) -m 644 ${<} ${@}

${INSTALL_ROOT}/lib/libc.a: ${CURDIR}/libc.a
	@echo "INSTALLING ${@}"
	mkdir -p $(@D)
	$(INSTALL) -m 644 ${<} ${@}

${INSTALL_ROOT}/include/%.h: ${PROJECT_ROOT}/include/%.h
	@echo "INSTALLING ${@}"
	mkdir -p $(@D)
	$(INSTALL) -m 644 ${<} ${@}

endif
