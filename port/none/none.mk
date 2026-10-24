#
# Copyright (C) 2026 Red Rocket Computing, LLC
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
# none.mk
#
# Created on: Oct 18, 2026
#     Author: Stephen Street (stephen@redrocketcomputing.com)
#

ifeq ($(findstring ${BUILD_ROOT},${CURDIR}),)
SUBDIRS := ${ARCH_FAMILY}
include ${PROJECT_ROOT}/tools/makefiles/target.mk
else

# librtos-none.a: the bare-metal single-threaded port, generic part plus the ${ARCH_FAMILY} variant
TARGET_OBJ_LIBS := port/none/${ARCH_FAMILY}

EXTRA_DEPS := ${SOURCE_DIR}/none.mk

include ${PROJECT_ROOT}/tools/makefiles/project.mk

CFLAGS += ${LIBC_CFLAGS}

all: ${INSTALL_ROOT}/lib/librtos-none.a

${INSTALL_ROOT}/lib/librtos-none.a: ${CURDIR}/librtos-none.a
	@echo "INSTALLING ${@}"
	mkdir -p $(@D)
	$(INSTALL) -m 644 ${<} ${@}

endif
