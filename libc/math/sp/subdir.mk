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

# The layer's functions, less those a later FPU layer of this core provides in an instruction (D46, D56), and in the
# fast flavour those of the fast/ directories (D162, common.mk)
SP_OVERRIDES := $(foreach layer,$(filter-out sp,${LIBC_MATH_LAYERS}),$(notdir $(wildcard ${SOURCE_DIR}/../${layer}/*.c)))
SRC := $(filter-out $(addprefix ${SOURCE_DIR}/,${SP_OVERRIDES}),$(wildcard ${SOURCE_DIR}/*.c))
SRC := $(call libc_math_fast_filter,${SRC})
