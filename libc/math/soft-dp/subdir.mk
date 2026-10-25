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

# The helpers of this layer, less those a layer listed after it in LIBC_MATH_LAYERS replaces with a file of the
# same name (.c or .S): soft-dp-nodsp's __aeabi_dmul.S on armv7-m, which has no UMAAL (PLAN D108)
libc_math_after = $(if $(2),$(if $(filter $(1),$(firstword $(2))),$(wordlist 2,$(words $(2)),$(2)),$(call libc_math_after,$(1),$(wordlist 2,$(words $(2)),$(2)))))
SOFT_DP_LATER := $(call libc_math_after,soft-dp,${LIBC_MATH_LAYERS})
SOFT_DP_REPLACED := $(foreach layer,${SOFT_DP_LATER},$(basename $(notdir $(wildcard ${SOURCE_DIR}/../${layer}/*.c ${SOURCE_DIR}/../${layer}/*.S))))
SRC := $(filter-out $(foreach name,${SOFT_DP_REPLACED},${SOURCE_DIR}/${name}.c ${SOURCE_DIR}/${name}.S),$(wildcard ${SOURCE_DIR}/*.c ${SOURCE_DIR}/*.S))
