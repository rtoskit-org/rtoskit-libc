#
# Copyright (C) 2026 Red Rocket Computing, LLC
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
# subdir.mk
#
# Created on: Oct 20, 2026
#     Author: Stephen Street (stephen@redrocketcomputing.com)
#

# The generic files, less those the speed flavour replaces with the file of the same name in speed/ (PLAN D77); the
# fast flavour falls back to them (LIBC_FLAVOURS, D162)
LIBC_SPEED_OVERRIDES := $(if $(filter speed,${LIBC_FLAVOURS}),$(notdir $(wildcard ${SOURCE_DIR}/speed/*.c)))
SRC := $(filter-out $(addprefix ${SOURCE_DIR}/,${LIBC_SPEED_OVERRIDES}),$(wildcard ${SOURCE_DIR}/*.c))
