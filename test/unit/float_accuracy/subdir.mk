#
# Copyright (C) 2026 Red Rocket Computing, LLC
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
# subdir.mk
#
# Created on: Nov 10, 2026
#     Author: Stephen Street (stephen@redrocketcomputing.com)
#

# The accuracy of the build type's contract (PLAN D162, D164): expect.c in debug, release and speed, fast/expect.c in fast
SRC := $(filter-out ${SOURCE_DIR}/expect.c,$(wildcard ${SOURCE_DIR}/*.c))
SRC += $(if $(filter fast,${LIBC_FLAVOURS}),${SOURCE_DIR}/fast/expect.c,${SOURCE_DIR}/expect.c)
