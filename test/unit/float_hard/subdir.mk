#
# Copyright (C) 2026 Red Rocket Computing, LLC
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
# subdir.mk
#
# Created on: Oct 22, 2026
#     Author: Stephen Street (stephen@redrocketcomputing.com)
#

# The double path of the float functions, with every rounding direction (PLAN D44, D98)
ifneq ($(filter sse2 fpv5-dp,${LIBC_FPU}),)
SRC += ${SOURCE_DIR}/float_hard.c
# The expectation of the build type's contract: the strict one, or in fast the fast one (PLAN D162, D164)
SRC += $(if $(filter fast,${LIBC_FLAVOURS}),${SOURCE_DIR}/fast/expect.c,${SOURCE_DIR}/expect.c)
endif
