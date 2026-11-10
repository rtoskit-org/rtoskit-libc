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

# The generic fast files, less those a layer's fast/ directory replaces with a file of the same name (PLAN D162)
LIBC_MATH_FAST_LAYERED := $(foreach layer,${LIBC_MATH_LAYERS},$(basename $(notdir $(wildcard ${SOURCE_DIR}/../${layer}/fast/*.c ${SOURCE_DIR}/../${layer}/fast/*.S))))
SRC := $(filter-out $(foreach stem,${LIBC_MATH_FAST_LAYERED},${SOURCE_DIR}/${stem}.c ${SOURCE_DIR}/${stem}.S),$(wildcard ${SOURCE_DIR}/*.c ${SOURCE_DIR}/*.S))
