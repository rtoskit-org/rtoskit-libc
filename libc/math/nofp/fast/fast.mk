#
# Copyright (C) 2026 Red Rocket Computing, LLC
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
# fast.mk
#
# Created on: Nov 11, 2026
#     Author: Stephen Street (stephen@redrocketcomputing.com)
#

# The fast flavour's fixed-point float functions for the cores without an FPU (PLAN D162, D172;
# docs/research/libm-fast-float-exp-log.md section 15): nofp/speed's files without the rounding test, and new ones over
# the same cores. Built in every build type, archived into libc.a in fast only where the arch file lists nofp in
# LIBC_MATH_FAST_LAYERS (libc/libc.mk), where each replaces the nofp/speed, sp-nearest and generic file of its name

ifeq ($(findstring ${BUILD_ROOT},${CURDIR}),)
include ${PROJECT_ROOT}/tools/makefiles/target.mk
else

include ${PROJECT_ROOT}/tools/makefiles/project.mk

CPPFLAGS += -I${PROJECT_ROOT}/libc
CFLAGS += ${LIBC_CFLAGS} -fno-math-errno

# nofp/speed's fixed-point headers and its 32 x 32 -> 64 multiply for this core (UMULL inline on armv7-m, the 16-bit
# product helper on Armv6-M)
NOFP_WIDE := $(if $(filter armv6-m armv6s-m,${ARCH_CROSS}),muls,umull)
CPPFLAGS += -I${SOURCE_DIR}/../speed -I${SOURCE_DIR}/../speed/${NOFP_WIDE}

# As the sp-nearest layer whose fallback paths these files keep (PLAN D56)
CFLAGS += -Wdouble-promotion

endif
