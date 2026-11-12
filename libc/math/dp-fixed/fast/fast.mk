#
# Copyright (C) 2026 Red Rocket Computing, LLC
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
# fast.mk
#
# Created on: Nov 12, 2026
#     Author: Stephen Street (stephen@redrocketcomputing.com)
#

ifeq ($(findstring ${BUILD_ROOT},${CURDIR}),)
include ${PROJECT_ROOT}/tools/makefiles/target.mk
else

include ${PROJECT_ROOT}/tools/makefiles/project.mk

CPPFLAGS += -I${PROJECT_ROOT}/libc
CFLAGS += ${LIBC_CFLAGS} -fno-math-errno

# The 32 x 32 -> 64 product of the speed files (UMULL inline, four 16-bit products on Armv6-M, PLAN D136)
DP_FIXED_WIDE := $(if $(filter armv6-m armv6s-m,${ARCH_CROSS}),muls,umull)
CPPFLAGS += -I${SOURCE_DIR}/../speed -I${SOURCE_DIR}/../speed/${DP_FIXED_WIDE}

endif
