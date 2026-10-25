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

# The kernels of this directory, less those a directory listed before it in
# LIBC_STRING_KERNELS provides: the first directory with a function wins (PLAN D73)
libc_string_before = $(if $(2),$(if $(filter $(1),$(firstword $(2))),,$(firstword $(2)) $(call libc_string_before,$(1),$(wordlist 2,$(words $(2)),$(2)))))
LIBC_STRING_EARLIER := $(call libc_string_before,$(notdir ${SOURCE_DIR}),${LIBC_STRING_KERNELS})
LIBC_STRING_SHADOWED := $(foreach kernels,${LIBC_STRING_EARLIER},$(notdir $(wildcard ${SOURCE_DIR}/../${kernels}/*.S)))
SRC := $(filter-out $(addprefix ${SOURCE_DIR}/,${LIBC_STRING_SHADOWED}),$(wildcard ${SOURCE_DIR}/*.S))
