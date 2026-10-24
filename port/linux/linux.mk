#
# Copyright (C) 2026 Red Rocket Computing, LLC
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
# linux.mk
#
# Created on: Oct 18, 2026
#     Author: Stephen Street (stephen@redrocketcomputing.com)
#

ifeq ($(findstring ${BUILD_ROOT},${CURDIR}),)
SUBDIRS := ${ARCH_FAMILY}
include ${PROJECT_ROOT}/tools/makefiles/target.mk
else

# librtos-linux.a: the Linux test port, generic part plus the ${ARCH_FAMILY} variant
TARGET_OBJ_LIBS := port/linux/${ARCH_FAMILY}

EXTRA_DEPS := ${SOURCE_DIR}/linux.mk

include ${PROJECT_ROOT}/tools/makefiles/project.mk

CPPFLAGS += -I${SOURCE_DIR}/${ARCH_FAMILY}
CFLAGS += ${LIBC_CFLAGS}

all: ${INSTALL_ROOT}/lib/librtos-linux.a

${INSTALL_ROOT}/lib/librtos-linux.a: ${CURDIR}/librtos-linux.a
	@echo "INSTALLING ${@}"
	mkdir -p $(@D)
	$(INSTALL) -m 644 ${<} ${@}

endif
