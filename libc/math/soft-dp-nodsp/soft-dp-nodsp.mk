#
# Copyright (C) 2026 Red Rocket Computing, LLC
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
# soft-dp-nodsp.mk
#
# Created on: Oct 25, 2026
#     Author: Stephen Street (stephen@redrocketcomputing.com)
#

# soft-dp's helpers rewritten without the DSP extension, for armv7-m; listed after soft-dp, whose subdir.mk then
# drops the files of the same name (PLAN D108)
ifeq ($(findstring ${BUILD_ROOT},${CURDIR}),)
include ${PROJECT_ROOT}/tools/makefiles/target.mk
else

include ${PROJECT_ROOT}/tools/makefiles/project.mk

CPPFLAGS += -I${PROJECT_ROOT}/libc
CFLAGS += ${LIBC_CFLAGS}

endif
