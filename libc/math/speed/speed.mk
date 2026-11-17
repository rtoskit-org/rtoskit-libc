#
# Copyright (C) 2026 Red Rocket Computing, LLC
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
# speed.mk
#
# Created on: Nov 17, 2026
#     Author: Stephen Street (stephen@redrocketcomputing.com)
#

# The speed flavour's versions of generic libm files (PLAN D77, D228): built in every build, archived into libc.a
# only in the speed and fast flavours (libc/libc.mk), where they replace the generic file of the same name
# (libc/math/subdir.mk)

ifeq ($(findstring ${BUILD_ROOT},${CURDIR}),)
include ${PROJECT_ROOT}/tools/makefiles/target.mk
else

include ${PROJECT_ROOT}/tools/makefiles/project.mk

CPPFLAGS += -I${PROJECT_ROOT}/libc
CFLAGS += ${LIBC_CFLAGS} -frounding-math -fsignaling-nans -fno-math-errno

endif
