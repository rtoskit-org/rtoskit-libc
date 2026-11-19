#
# Copyright (C) 2026 Red Rocket Computing, LLC
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
# subdir.mk
#
# Created on: Nov 19, 2026
#     Author: Stephen Street (stephen@redrocketcomputing.com)
#

# The layer's integer sqrtf, less in the fast flavour the file of the same name in nofp/fast/ (D162, D242, common.mk)
SRC := $(call libc_math_fast_filter,$(wildcard ${SOURCE_DIR}/*.c),nofp)
