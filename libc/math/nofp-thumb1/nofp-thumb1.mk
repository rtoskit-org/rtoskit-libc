#
# Copyright (C) 2026 Red Rocket Computing, LLC
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
# nofp-thumb1.mk
#
# Created on: Oct 25, 2026
#     Author: Stephen Street (stephen@redrocketcomputing.com)
#

# The soft-float helpers that replace libgcc's on Armv6-M (PLAN D55, D85, D108). libgcc's Armv6-M helpers are one
# function per object; on Armv7-M they come in groups (fadd with fsub), so these would clash there and only the
# Thumb-1 cores list this layer. softfloat.h stays in libc/math/nofp.
ifeq ($(findstring ${BUILD_ROOT},${CURDIR}),)
include ${PROJECT_ROOT}/tools/makefiles/target.mk
else

include ${PROJECT_ROOT}/tools/makefiles/project.mk

CPPFLAGS += -I${PROJECT_ROOT}/libc -I${PROJECT_ROOT}/libc/math/nofp
CFLAGS += ${LIBC_CFLAGS} -frounding-math -fsignaling-nans -fno-math-errno

endif
