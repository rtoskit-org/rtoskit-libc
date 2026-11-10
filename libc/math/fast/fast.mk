#
# Copyright (C) 2026 Red Rocket Computing, LLC
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
# fast.mk
#
# Created on: Nov 10, 2026
#     Author: Stephen Street (stephen@redrocketcomputing.com)
#

# The fast flavour's generic libm files (PLAN D162, D163): built in every build type, archived into libc.a only when
# LIBC_FLAVOURS has fast (libc/libc.mk), where each replaces the generic, layer and speed/ file of the same name
# (LIBC_MATH_FAST_STEMS, tools/makefiles/common.mk). A layer's own libc/math/<layer>/fast/ file replaces one here
# (subdir.mk); a <layer>/fast/ module drops the stems of the fast/ directories of the layers after it.

ifeq ($(findstring ${BUILD_ROOT},${CURDIR}),)
include ${PROJECT_ROOT}/tools/makefiles/target.mk
else

include ${PROJECT_ROOT}/tools/makefiles/project.mk

CPPFLAGS += -I${PROJECT_ROOT}/libc
CFLAGS += ${LIBC_CFLAGS}

# Round to nearest only (D163): no -frounding-math; signaling NaNs and trapping math stay, since C23 7.12.1p1 forbids
# spurious invalid, divide-by-zero and overflow; errors are reported through exceptions only (D45)
CFLAGS += -fsignaling-nans -fno-math-errno

endif
