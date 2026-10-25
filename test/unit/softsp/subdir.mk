#
# Copyright (C) 2026 Red Rocket Computing, LLC
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
# subdir.mk
#
# Created on: Oct 25, 2026
#     Author: Stephen Street (stephen@redrocketcomputing.com)
#

# The helpers exist on the Arm cores only (PLAN D109); Armv6-M's libgcc flag-returning comparisons do not follow RTABI
ifeq (${ARCH_FAMILY},arm)
SRC := ${SOURCE_DIR}/softsp.c ${SOURCE_DIR}/operations.c ${SOURCE_DIR}/$(if $(filter armv6-m armv6s-m,${ARCH_CROSS}),cfcmp-thumb1.c,cfcmp.c)
else
SRC := ${SOURCE_DIR}/softsp.c ${SOURCE_DIR}/none.c
endif
