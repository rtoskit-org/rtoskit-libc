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

# The bounds of the build type's accuracy contract (PLAN D162, D163): bounds.c in debug, release and speed, fast/bounds.c
# in fast; the rest of the test is the same in every build type
SRC := $(filter-out ${SOURCE_DIR}/bounds.c,$(wildcard ${SOURCE_DIR}/*.c))
SRC += $(if $(filter fast,${LIBC_FLAVOURS}),${SOURCE_DIR}/fast/bounds.c,${SOURCE_DIR}/bounds.c)
