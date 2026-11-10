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

# The layer's functions, less those a later layer of this core provides (D46, D57), in the speed and fast flavours
# those a later layer's speed/ directory provides (D77, D86), and in the fast flavour those of the fast/ directories
# (D162, common.mk)
SP_NEAREST_LATER := $(filter-out sp sp-nearest,${LIBC_MATH_LAYERS})
SP_NEAREST_OVERRIDES := $(foreach layer,${SP_NEAREST_LATER},$(notdir $(wildcard ${SOURCE_DIR}/../${layer}/*.c)))
SP_NEAREST_OVERRIDES += $(if $(filter speed,${LIBC_FLAVOURS}),$(foreach layer,${SP_NEAREST_LATER},$(notdir $(wildcard ${SOURCE_DIR}/../${layer}/speed/*.c))))
SRC := $(filter-out $(addprefix ${SOURCE_DIR}/,${SP_NEAREST_OVERRIDES}),$(wildcard ${SOURCE_DIR}/*.c))
SRC := $(call libc_math_fast_filter,${SRC})
