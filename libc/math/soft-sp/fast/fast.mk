#
# Copyright (C) 2026 Red Rocket Computing, LLC
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
# fast.mk
#
# Created on: Nov 19, 2026
#     Author: Stephen Street (stephen@redrocketcomputing.com)
#

# armv7-m's own fast files (PLAN D242): soft-sp is the one layer only armv7-m lists, so a file here replaces the nofp/fast
# file of its name on that core alone, where UMULL makes a multiply-based kernel cheaper than nofp/fast's (Armv6-M's)
# choice. Built in every build type, archived in fast where armv7-m.mk lists soft-sp in LIBC_MATH_FAST_LAYERS

ifeq ($(findstring ${BUILD_ROOT},${CURDIR}),)
include ${PROJECT_ROOT}/tools/makefiles/target.mk
else

include ${PROJECT_ROOT}/tools/makefiles/project.mk

CPPFLAGS += -I${PROJECT_ROOT}/libc
CFLAGS += ${LIBC_CFLAGS} -fno-math-errno

# nofp/fast's and nofp/speed's fixed-point headers and the 32 x 32 -> 64 multiply (UMULL inline on armv7-m)
NOFP_WIDE := $(if $(filter armv6-m armv6s-m,${ARCH_CROSS}),muls,umull)
CPPFLAGS += -I${PROJECT_ROOT}/libc/math/nofp/fast -I${PROJECT_ROOT}/libc/math/nofp/speed -I${PROJECT_ROOT}/libc/math/nofp/speed/${NOFP_WIDE}

# As the sp-nearest layer whose fallback paths these files keep (PLAN D56)
CFLAGS += -Wdouble-promotion

endif
