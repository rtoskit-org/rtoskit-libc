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

# The fast flavour's float files of the sp-nearest cores (PLAN D162, D168; docs/research/libm-fast-float-exp-log.md):
# float arithmetic only, no float-float, round to nearest only as the layer (D57). Built in every build type,
# archived into libc.a in fast only (libc/libc.mk), where each replaces the generic, sp and sp-nearest file of the same
# name but gives way to a later layer's (subdir.mk: the fixed-point nofp/speed files on the cores without an FPU)

ifeq ($(findstring ${BUILD_ROOT},${CURDIR}),)
include ${PROJECT_ROOT}/tools/makefiles/target.mk
else

EXTRA_TARGETS := ${CURDIR}/fast.checked

include ${PROJECT_ROOT}/tools/makefiles/project.mk

CPPFLAGS += -I${PROJECT_ROOT}/libc
CFLAGS += ${LIBC_CFLAGS}

# Round to nearest only (D163): no -frounding-math; signaling NaNs and trapping math stay; exceptions only (D45)
CFLAGS += -fsignaling-nans -fno-math-errno

# Float only, as the layer (PLAN D56, D57): warn on every promotion to double, and refuse any double helper
CFLAGS += -Wdouble-promotion
FAST_FORBIDDEN := __aeabi_(d[a-z0-9]+|f2d|f2u?lz|u?l2[df]|u?i2d)
${CURDIR}/fast.checked: ${OBJ}
	@echo "CHECKING ${@}"
	@if [ -n "${OBJ}" ] && ${NM} ${OBJ} | grep -E ' U ${FAST_FORBIDDEN}$$'; then echo "libc/math/sp-nearest/fast: double-precision helper referenced" >&2; exit 1; fi
	@touch $@

endif
