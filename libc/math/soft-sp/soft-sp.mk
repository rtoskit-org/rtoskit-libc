#
# Copyright (C) 2026 Red Rocket Computing, LLC
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
# soft-sp.mk
#
# Created on: Oct 25, 2026
#     Author: Stephen Street (stephen@redrocketcomputing.com)
#

# The single-precision run-time ABI helpers for armv7-m (PLAN D109): libgcc's grouped addition, multiplication and
# comparison objects replaced whole, and the libgcc helpers that call them; softfloat.h stays in libc/math/nofp
ifeq ($(findstring ${BUILD_ROOT},${CURDIR}),)
include ${PROJECT_ROOT}/tools/makefiles/target.mk
else

include ${PROJECT_ROOT}/tools/makefiles/project.mk

CPPFLAGS += -I${PROJECT_ROOT}/libc -I${PROJECT_ROOT}/libc/math/nofp
CFLAGS += ${LIBC_CFLAGS}

endif
