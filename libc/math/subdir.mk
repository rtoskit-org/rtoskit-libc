#
# Copyright (C) 2026 Red Rocket Computing, LLC
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
# subdir.mk
#
# Created on: Oct 19, 2026
#     Author: Stephen Street (stephen@redrocketcomputing.com)
#

# The generic functions, less those an FPU layer of this core provides in an instruction (D46)
LIBC_MATH_OVERRIDES := $(foreach layer,${LIBC_MATH_LAYERS},$(notdir $(wildcard ${SOURCE_DIR}/${layer}/*.c)))
# ... and in the speed and fast flavours those a layer's speed/ directory replaces (D77; dp-fixed's sin, cos and sincos,
# P8.03)
LIBC_MATH_OVERRIDES += $(if $(filter speed,${LIBC_FLAVOURS}),$(foreach layer,${LIBC_MATH_LAYERS},$(notdir $(wildcard ${SOURCE_DIR}/${layer}/speed/*.c))))
SRC := $(filter-out $(addprefix ${SOURCE_DIR}/,${LIBC_MATH_OVERRIDES}),$(wildcard ${SOURCE_DIR}/*.c))
# ... and in the fast flavour those of libc/math/fast/ and the layers' fast/ directories (D162, common.mk)
SRC := $(call libc_math_fast_filter,${SRC})
