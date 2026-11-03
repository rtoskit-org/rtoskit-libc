#
# Copyright (C) 2026 Red Rocket Computing, LLC
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
# speed.mk
#
# Created on: Nov 3, 2026
#     Author: Stephen Street (stephen@redrocketcomputing.com)
#

# The speed flavour's fixed-point double sin, cos and sincos for the cores without a double FPU (P8.03, PLAN
# D133, docs/research/sincos.md): built in every build of the layer, archived into libc.a only when LIBC_FLAVOUR is
# speed (libc/libc.mk), where they replace the generic files of the same name (libc/math/subdir.mk)

ifeq ($(findstring ${BUILD_ROOT},${CURDIR}),)
include ${PROJECT_ROOT}/tools/makefiles/target.mk
else

include ${PROJECT_ROOT}/tools/makefiles/project.mk

CPPFLAGS += -I${PROJECT_ROOT}/libc
CFLAGS += ${LIBC_CFLAGS} -frounding-math -fsignaling-nans -fno-math-errno

# The speed flavour's optimisation for its own files (D73, as __libc_scan.o has)
CFLAGS += ${LIBC_SPEED_CFLAGS}

# The 32 x 32 -> 64 multiply: UMULL inline where the core has it, the 16-bit product helper on Armv6-M (D22: a
# variant directory, not #if)
CPPFLAGS += -I${SOURCE_DIR}/$(if $(filter armv6-m armv6s-m,${ARCH_CROSS}),muls,umull)

endif
