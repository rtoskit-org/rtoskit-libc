#
# Copyright (C) 2026 Red Rocket Computing, LLC
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
# speed.mk
#
# Created on: Oct 21, 2026
#     Author: Stephen Street (stephen@redrocketcomputing.com)
#

# The speed flavour's fixed-point float functions for the cores without an FPU (PLAN D77, D86): built in
# every build of the nofp layer, archived into libc.a only when LIBC_FLAVOUR is speed (libc/libc.mk), where
# they replace the sp-nearest files of the same name (libc/math/sp-nearest/subdir.mk)

ifeq ($(findstring ${BUILD_ROOT},${CURDIR}),)
include ${PROJECT_ROOT}/tools/makefiles/target.mk
else

include ${PROJECT_ROOT}/tools/makefiles/project.mk

CPPFLAGS += -I${PROJECT_ROOT}/libc
CFLAGS += ${LIBC_CFLAGS} -fno-math-errno

# The 32 x 32 -> 64 multiply: UMULL inline on armv7-m, the 16-bit product helper on Armv6-M (P9.01, Q43; a variant
# directory, not #if)
NOFP_WIDE := $(if $(filter armv6-m armv6s-m,${ARCH_CROSS}),muls,umull)
CPPFLAGS += -I${SOURCE_DIR}/${NOFP_WIDE}

# sinf, cosf and sincosf call the D136 kernel (P9.02, Q41): its header, and its multiply for this core
CPPFLAGS += -I${SOURCE_DIR}/../../dp-fixed/speed -I${SOURCE_DIR}/../../dp-fixed/speed/${NOFP_WIDE}

# As the sp-nearest layer whose fallback paths these files keep (PLAN D56)
CFLAGS += -Wdouble-promotion

endif
