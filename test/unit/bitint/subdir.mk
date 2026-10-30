#
# Copyright (C) 2026 Red Rocket Computing, LLC
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
# subdir.mk
#
# Created on: Oct 30, 2026
#     Author: Stephen Street (stephen@redrocketcomputing.com)
#

# Only where the compiler has _BitInt for the architecture, which it shows by predefining
# __BITINT_MAXWIDTH__; GCC 14 and 15 on 32-bit Arm do not (PLAN D127). The test is then
# reported as skipped, not built
BITINT_MAXWIDTH_PROBE := $(shell echo __BITINT_MAXWIDTH__ | ${CC} ${CROSS_FLAGS} -std=c23 -E -P -x c - 2> /dev/null)
ifneq (${BITINT_MAXWIDTH_PROBE},__BITINT_MAXWIDTH__)
SRC += ${SOURCE_DIR}/bitint.c
endif
