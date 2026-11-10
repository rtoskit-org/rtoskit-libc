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

# The layer's fast files, less those a later layer of the core provides, plain, in speed/ or in fast/ (PLAN D168)
SP_NEAREST_FAST_LATER := $(call libc_math_layer_stems,$(call libc_math_later,sp-nearest))
SRC := $(filter-out $(foreach stem,${SP_NEAREST_FAST_LATER},${SOURCE_DIR}/${stem}.c ${SOURCE_DIR}/${stem}.S),$(wildcard ${SOURCE_DIR}/*.c ${SOURCE_DIR}/*.S))
