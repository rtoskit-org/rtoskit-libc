#
# Copyright (C) 2026 Red Rocket Computing, LLC
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
# math.mk
#
# Created on: Oct 19, 2026
#     Author: Stephen Street (stephen@redrocketcomputing.com)
#

ifeq ($(findstring ${BUILD_ROOT},${CURDIR}),)
# The FPU layers, and the fast flavour's generic files (built in every build type, archived in fast only, D162)
SUBDIRS := ${LIBC_MATH_LAYERS} fast
include ${PROJECT_ROOT}/tools/makefiles/target.mk
else

include ${PROJECT_ROOT}/tools/makefiles/project.mk

CPPFLAGS += -I${PROJECT_ROOT}/libc
CFLAGS += ${LIBC_CFLAGS}

# Honour the dynamic rounding direction and signaling NaNs; errors are reported through exceptions only (D45)
CFLAGS += -frounding-math -fsignaling-nans -fno-math-errno

endif
